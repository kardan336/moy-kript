#pragma once
#include <vector>
#include <cstdint>
#include <string>
std::vector<uint8_t> AdvancedEncrypt(const std::vector<uint8_t>& data, const std::string& password);
std::vector<uint8_t> AdvancedDecrypt(const std::vector<uint8_t>& data, const std::string& password);