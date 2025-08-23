#include "WinCryptor.h"
#include <algorithm>

WinCryptor::WinCryptor() {}
WinCryptor::~WinCryptor() {}

std::string WinCryptor::encrypt(const std::string& data) {
    std::string result = data;
    std::reverse(result.begin(), result.end());
    return result;
}

std::string WinCryptor::decrypt(const std::string& data) {
    // Т.к. наш "шифр" — это разворот строки, то и дешифрование — это разворот
    return encrypt(data);
}
