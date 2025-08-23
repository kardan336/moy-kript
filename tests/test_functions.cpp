#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

// Copy the enhanced functions we want to test from randomized_stub.cpp
std::string RandomString(size_t len) {
    static const char abc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string s;
    std::random_device rd; std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(0, sizeof(abc)-2);
    for (size_t i=0; i<len; ++i) s += abc[dist(mt)];
    return s;
}

std::string GenerateJunkCode(int lines) {
    std::string junk;
    std::vector<std::string> junkTemplates = {
        "volatile int junk%d = %d;\n",
        "junk%d += %d;\n",
        "for(int k=0;k<%d;++k) junk%d+=k;\n",
        "SetLastError(%d);\n",
        "DWORD tick%d = GetTickCount();\n",
        "volatile DWORD dummy%d = tick%d ^ %d;\n",
        "if (dummy%d %% 2) tick%d++;\n",
        "SYSTEMTIME st%d; GetSystemTime(&st%d);\n",
        "volatile int calc%d = st%d.wYear * %d;\n"
    };
    
    for (int i = 0; i < lines; ++i) {
        int templateIdx = rand() % junkTemplates.size();
        char buffer[256];
        
        switch (templateIdx) {
            case 0: // volatile int
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, rand() % 1000000);
                break;
            case 1: // junk += 
                if (i > 0) sprintf(buffer, junkTemplates[templateIdx].c_str(), i-1, rand() % 200);
                else continue;
                break;
            case 2: // for loop
                if (i > 0) sprintf(buffer, junkTemplates[templateIdx].c_str(), rand() % 10 + 1, i-1);
                else continue;
                break;
            case 3: // SetLastError
                sprintf(buffer, junkTemplates[templateIdx].c_str(), rand() % 1000);
                break;
            case 4: // GetTickCount
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i);
                break;
            case 5: // dummy calculation
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i, rand() % 0xFFFF);
                break;
            case 6: // conditional
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i);
                break;
            case 7: // SYSTEMTIME
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i);
                break;
            case 8: // calculation with time
                sprintf(buffer, junkTemplates[templateIdx].c_str(), i, i, rand() % 100 + 1);
                break;
        }
        junk += buffer;
    }
    return junk;
}

std::string GenerateEnhancedAntiDebugCode() {
    return R"(
        // Enhanced anti-debug with multiple techniques
        if (IsDebuggerPresent()) return true;
        BOOL bDebugger = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &bDebugger);
        if (bDebugger) return true;
        
        // PEB checks
        #ifdef _WIN64
        PPEB pPEB = (PPEB)__readgsqword(0x60);
        #else
        PPEB pPEB = (PPEB)__readfsdword(0x30);
        #endif
        if (pPEB && pPEB->BeingDebugged) return true;
        if (pPEB && (pPEB->NtGlobalFlag & 0x70)) return true;
        
        return false;
    )";
}

std::string BytesToHexArray(const std::vector<uint8_t>& data) {
    if (data.empty()) return "";
    
    std::ostringstream oss;
    for (size_t i=0; i<data.size(); ++i) {
        oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)data[i];
        if (i+1 != data.size()) oss << ",";
        
        // Prevent too long lines
        if (i > 0 && (i+1) % 16 == 0 && i+1 != data.size()) {
            oss << "\n        ";
        }
    }
    return oss.str();
}

int main() {
    std::cout << "Testing enhanced stub generation functions..." << std::endl;
    
    // Test RandomString with different lengths
    for (int len : {8, 16, 32}) {
        std::string randomStr = RandomString(len);
        std::cout << "Random string (" << len << " chars): " << randomStr << std::endl;
        if (randomStr.length() != len) {
            std::cout << "FAILED: Wrong length" << std::endl;
            return 1;
        }
    }
    
    // Test uniqueness of random strings
    std::vector<std::string> randomStrings;
    for (int i = 0; i < 10; ++i) {
        randomStrings.push_back(RandomString(16));
    }
    
    for (size_t i = 0; i < randomStrings.size(); ++i) {
        for (size_t j = i + 1; j < randomStrings.size(); ++j) {
            if (randomStrings[i] == randomStrings[j]) {
                std::cout << "WARNING: Duplicate random strings found" << std::endl;
            }
        }
    }
    
    // Test enhanced junk code generation
    std::string junkCode = GenerateJunkCode(10);
    std::cout << "Enhanced junk code (10 lines):" << std::endl << junkCode << std::endl;
    if (junkCode.empty()) {
        std::cout << "FAILED: No junk code generated" << std::endl;
        return 1;
    }
    
    // Verify junk code has variety
    if (junkCode.find("volatile") == std::string::npos &&
        junkCode.find("GetTickCount") == std::string::npos &&
        junkCode.find("SetLastError") == std::string::npos) {
        std::cout << "WARNING: Junk code lacks variety" << std::endl;
    }
    
    // Test enhanced anti-debug code
    std::cout << "Enhanced anti-debug code:" << std::endl << GenerateEnhancedAntiDebugCode() << std::endl;
    
    // Test hex array generation with proper formatting
    std::vector<uint8_t> testData;
    for (int i = 0; i < 48; ++i) {
        testData.push_back(i % 256);
    }
    
    std::string hexArray = BytesToHexArray(testData);
    std::cout << "Hex array (48 bytes with formatting):" << std::endl << hexArray << std::endl;
    
    // Verify formatting includes newlines
    if (testData.size() > 16 && hexArray.find('\n') == std::string::npos) {
        std::cout << "WARNING: Long hex array should have line breaks" << std::endl;
    }
    
    // Test template substitution with random function names
    std::string template_code = "void /*FUNC1*/() { /*JUNK1*/ }\nvoid /*FUNC2*/() { /*JUNK2*/ }";
    
    std::vector<std::string> funcNames = {
        "Func_" + RandomString(8),
        "Func_" + RandomString(8)
    };
    
    // Simulate substitution
    size_t pos;
    while ((pos = template_code.find("/*FUNC1*/")) != std::string::npos) {
        template_code.replace(pos, 9, funcNames[0]);
    }
    while ((pos = template_code.find("/*FUNC2*/")) != std::string::npos) {
        template_code.replace(pos, 9, funcNames[1]);
    }
    while ((pos = template_code.find("/*JUNK1*/")) != std::string::npos) {
        template_code.replace(pos, 9, GenerateJunkCode(2));
    }
    while ((pos = template_code.find("/*JUNK2*/")) != std::string::npos) {
        template_code.replace(pos, 9, GenerateJunkCode(2));
    }
    
    std::cout << "Template substitution with random functions:" << std::endl << template_code << std::endl;
    
    // Test empty data edge case
    std::vector<uint8_t> emptyData;
    std::string emptyHex = BytesToHexArray(emptyData);
    if (!emptyHex.empty()) {
        std::cout << "FAILED: Empty data should produce empty hex string" << std::endl;
        return 1;
    }
    
    std::cout << "All enhanced tests passed!" << std::endl;
    return 0;
}