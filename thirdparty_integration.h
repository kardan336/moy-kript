#pragma once
#include <string>

bool RunConfuserEx(const std::wstring& exePath, const std::wstring& crprojPath, std::wstring* log = nullptr);
bool RunEnigmaProtector(const std::wstring& exePath, std::wstring* log = nullptr);
bool RunDotNetToJScript(const std::wstring& dllPath, const std::wstring& jscriptOut, std::wstring* log = nullptr);