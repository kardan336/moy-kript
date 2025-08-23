#include "crypt_ext.h"
#include <algorithm>

std::vector<uint8_t> AdvancedEncrypt(const std::vector<uint8_t>& data, const std::string& password) {
    std::vector<uint8_t> out = data;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = out[i] ^ password[(i * 13) % password.size()] ^ (uint8_t)(i % 251);
    std::reverse(out.begin(), out.end());
    return out;
}

std::vector<uint8_t> AdvancedDecrypt(const std::vector<uint8_t>& data, const std::string& password) {
    std::vector<uint8_t> out = data;
    std::reverse(out.begin(), out.end());
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = out[i] ^ password[(i * 13) % password.size()] ^ (uint8_t)(i % 251);
    return out;
}