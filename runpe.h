#pragma once
#include <vector>
#include <string>
bool RunPE(const std::wstring& targetPath, const std::vector<uint8_t>& image, std::wstring* error = nullptr);