#ifndef WINCRYPTOR_H
#define WINCRYPTOR_H

#include <string>

class WinCryptor {
public:
    WinCryptor();
    ~WinCryptor();

    std::string encrypt(const std::string& data);
    std::string decrypt(const std::string& data);
};

#endif // WINCRYPTOR_H
