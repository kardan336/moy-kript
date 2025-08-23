#pragma once
#include <vector>
#include <cstdint>

// Генерирует рандомную оболочку со встроенным payload и запуском
bool GenerateRandomizedStub(const std::vector<uint8_t>& exe, std::vector<uint8_t>& outStub);