#pragma once
#include <vector>
#include <string>
bool ProcessHollowing(const std::wstring& targetPath, const std::vector<uint8_t>& payload, std::wstring* error = nullptr);