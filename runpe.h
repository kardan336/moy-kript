#pragma once
#include <vector>
#include <string>
#include <windows.h>

// RunPE инъекция с логированием
bool RunPE(const std::wstring& targetPath, const std::wstring& payloadPath, std::wstring* error = nullptr);

// RunPE инъекция из буфера данных  
bool RunPE(const std::wstring& targetPath, const std::vector<uint8_t>& image, std::wstring* error = nullptr);

// Вспомогательные функции
bool InjectToProcess(HANDLE hProcess, const std::vector<uint8_t>& image, std::wstring* error = nullptr);
std::vector<uint8_t> ReadFileToBuffer(const std::wstring& path);