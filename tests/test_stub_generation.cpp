#include "../randomized_stub.h"
#include <iostream>

// Forward declare the functions we want to test (they're currently Windows-only)
std::string RandomString(size_t len);
std::string GenerateJunkCode(int lines);

// Mock some basic functionality for testing
#ifndef _WIN32
std::string GenerateAntiDebugCode() { return "// Anti-debug placeholder"; }
std::string GenerateAntiVMCode() { return "// Anti-VM placeholder"; }
std::string GenerateAntiSandboxCode() { return "// Anti-sandbox placeholder"; }
std::string GenerateAntiTimingCode() { return "// Anti-timing placeholder"; }
std::string BytesToHexArray(const std::vector<uint8_t>& data) { return "0x00"; }
#endif

int main() {
    std::cout << "Testing string generation functions..." << std::endl;
    
    // Test RandomString
    std::string randomStr = RandomString(16);
    std::cout << "Random string (16 chars): " << randomStr << std::endl;
    if (randomStr.length() != 16) {
        std::cout << "FAILED: Wrong length" << std::endl;
        return 1;
    }
    
    // Test junk code generation
    std::string junkCode = GenerateJunkCode(5);
    std::cout << "Junk code (5 lines):" << std::endl << junkCode << std::endl;
    if (junkCode.empty()) {
        std::cout << "FAILED: No junk code generated" << std::endl;
        return 1;
    }
    
    // Test anti-analysis code generation
    std::cout << "Anti-debug code:" << std::endl << GenerateAntiDebugCode() << std::endl;
    std::cout << "Anti-VM code:" << std::endl << GenerateAntiVMCode() << std::endl;
    std::cout << "Anti-sandbox code:" << std::endl << GenerateAntiSandboxCode() << std::endl;
    std::cout << "Anti-timing code:" << std::endl << GenerateAntiTimingCode() << std::endl;
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}