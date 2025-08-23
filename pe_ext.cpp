#include "pe_ext.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

bool AnalyzePE(const std::vector<uint8_t>& exe, std::wstring& report) {
    if (exe.size() < 0x40) return false;
    std::wstringstream ss;
    ss << L"File size: " << exe.size() << L" bytes\n";
    ss << L"First bytes: ";
    for (size_t i = 0; i < 8 && i < exe.size(); ++i)
        ss << std::hex << std::setw(2) << std::setfill(L'0') << (int)exe[i] << L" ";
    report = ss.str();
    return true;
}

std::wstring GetPEHash(const std::vector<uint8_t>& exe) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(exe.data(), exe.size(), hash);
    std::wstringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill(L'0') << (int)hash[i];
    return ss.str();
}