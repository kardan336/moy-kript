#include "poly_engine.h"

PolyEngine::PolyEngine() {}

std::vector<uint8_t> PolyEngine::Encrypt(const std::vector<uint8_t>& data, const std::string& key) {
    std::vector<uint8_t> result = data;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] ^= key[i % key.size()];
    return result;
}

std::vector<uint8_t> PolyEngine::Decrypt(const std::vector<uint8_t>& data, const std::string& key) {
    return Encrypt(data, key); // XOR is symmetric
}