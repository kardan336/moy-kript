#pragma once
#include <vector>
#include <string>
bool AnalyzePE(const std::vector<uint8_t>& exe, std::wstring& report);
std::wstring GetPEHash(const std::vector<uint8_t>& exe);