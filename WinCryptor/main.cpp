#include <iostream>
#include "WinCryptor.h"

int main() {
    WinCryptor cryptor;
    std::string msg = "Hello, WinCryptor!";
    std::string enc = cryptor.encrypt(msg);
    std::string dec = cryptor.decrypt(enc);

    std::cout << "Original: " << msg << std::endl;
    std::cout << "Encrypted: " << enc << std::endl;
    std::cout << "Decrypted: " << dec << std::endl;
    return 0;
}