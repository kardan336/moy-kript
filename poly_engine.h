#pragma once
#include <vector>
#include <cstdint>

class PolyEngine {
public:
    PolyEngine();
    std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data, const std::string& key);
};