#include "thirdparty_integration.h"
#include <windows.h>

bool RunConfuserEx(const std::wstring& exePath, const std::wstring& crprojPath, std::wstring* log) {
    std::wstring command = L"tools\\ConfuserEx\\Confuser.CLI.exe -n \"" + crprojPath + L"\"";
    int res = (int)ShellExecuteW(NULL, L"open", L"tools\\ConfuserEx\\Confuser.CLI.exe", (L"-n \"" + crprojPath + L"\"").c_str(), NULL, SW_SHOW);
    if (log) *log = (res > 32) ? L"ConfuserEx started" : L"Failed to start ConfuserEx";
    return res > 32;
}

bool RunEnigmaProtector(const std::wstring& exePath, std::wstring* log) {
    int res = (int)ShellExecuteW(NULL, L"open", L"tools\\EnigmaProtector\\Enigma.exe", exePath.c_str(), NULL, SW_SHOW);
    if (log) *log = (res > 32) ? L"Enigma Protector started" : L"Failed to start Enigma Protector";
    return res > 32;
}

bool RunDotNetToJScript(const std::wstring& dllPath, const std::wstring& jscriptOut, std::wstring* log) {
    std::wstring args = L"tools\\DotNetToJScript\\DotNetToJScript.py \"" + dllPath + L"\" --output \"" + jscriptOut + L"\"";
    int res = (int)ShellExecuteW(NULL, L"open", L"python.exe", args.c_str(), NULL, SW_SHOW);
    if (log) *log = (res > 32) ? L"DotNetToJScript started" : L"Failed to start DotNetToJScript";
    return res > 32;
}