#pragma once
#include <vector>
#include <string>
bool PatchAVSignatures(std::vector<uint8_t>& exeData, std::wstring* log = nullptr);
bool RemoveKnownSections(std::vector<uint8_t>& exeData, std::wstring* log = nullptr);