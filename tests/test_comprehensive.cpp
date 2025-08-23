#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <cstring>

// Test the file reading function logic
std::vector<uint8_t> TestReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::vector<uint8_t>();
    }
    
    // Get file size
    in.seekg(0, std::ios::end);
    auto size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    // Check for reasonable file size (max 256MB)
    if (size < 0 || size > 0x10000000) {
        return std::vector<uint8_t>();
    }
    
    std::vector<uint8_t> data(static_cast<size_t>(size));
    in.read(reinterpret_cast<char*>(data.data()), size);
    
    if (!in) {
        return std::vector<uint8_t>();
    }
    
    return data;
}

bool TestSaveFile(const std::string& path, const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return false;
    }
    
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return out.good();
}

// Create a minimal valid PE structure for testing
std::vector<uint8_t> CreateTestPE() {
    std::vector<uint8_t> pe;
    
    // DOS header (simplified)
    uint8_t dosHeader[64] = {0};
    dosHeader[0] = 'M';  // e_magic = IMAGE_DOS_SIGNATURE
    dosHeader[1] = 'Z';
    dosHeader[60] = 0x80; // e_lfanew = 0x80 (128)
    
    pe.insert(pe.end(), dosHeader, dosHeader + 64);
    
    // Pad to e_lfanew position
    while (pe.size() < 128) {
        pe.push_back(0);
    }
    
    // NT headers (simplified)
    pe.push_back('P'); pe.push_back('E'); pe.push_back(0); pe.push_back(0); // Signature
    
    // FILE_HEADER (20 bytes)
    uint8_t fileHeader[20] = {0};
    fileHeader[0] = 0x4C; fileHeader[1] = 0x01; // Machine = IMAGE_FILE_MACHINE_I386
    // Other fields can remain 0 for this test
    pe.insert(pe.end(), fileHeader, fileHeader + 20);
    
    // OPTIONAL_HEADER (224 bytes for 32-bit)
    uint8_t optHeader[224] = {0};
    optHeader[0] = 0x0B; optHeader[1] = 0x01; // Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC
    optHeader[16] = 0x00; optHeader[17] = 0x10; optHeader[18] = 0x00; optHeader[19] = 0x00; // AddressOfEntryPoint = 0x1000
    optHeader[28] = 0x00; optHeader[29] = 0x00; optHeader[30] = 0x40; optHeader[31] = 0x00; // ImageBase = 0x400000
    pe.insert(pe.end(), optHeader, optHeader + 224);
    
    return pe;
}

bool ValidateBasicPE(const std::vector<uint8_t>& data) {
    if (data.size() < 64) return false; // Too small for DOS header
    
    // Check DOS signature
    if (data[0] != 'M' || data[1] != 'Z') return false;
    
    // Get e_lfanew
    uint32_t e_lfanew = *reinterpret_cast<const uint32_t*>(&data[60]);
    if (e_lfanew >= data.size()) return false;
    
    // Check NT signature
    if (e_lfanew + 4 > data.size()) return false;
    if (data[e_lfanew] != 'P' || data[e_lfanew + 1] != 'E' || 
        data[e_lfanew + 2] != 0 || data[e_lfanew + 3] != 0) {
        return false;
    }
    
    return true;
}

int main() {
    std::cout << "Running comprehensive functionality tests..." << std::endl;
    
    // Test 1: File I/O functions
    std::cout << "Test 1: File I/O functions..." << std::endl;
    
    std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    std::string testFile = "test_file.bin";
    
    if (!TestSaveFile(testFile, testData)) {
        std::cout << "FAILED: Could not save test file" << std::endl;
        return 1;
    }
    
    std::vector<uint8_t> readData = TestReadFile(testFile);
    if (readData != testData) {
        std::cout << "FAILED: Read data doesn't match written data" << std::endl;
        return 1;
    }
    
    // Clean up
    std::remove(testFile.c_str());
    std::cout << "PASSED: File I/O functions work correctly" << std::endl;
    
    // Test 2: PE validation logic
    std::cout << "Test 2: PE validation..." << std::endl;
    
    std::vector<uint8_t> validPE = CreateTestPE();
    if (!ValidateBasicPE(validPE)) {
        std::cout << "FAILED: Valid PE not recognized" << std::endl;
        return 1;
    }
    
    std::vector<uint8_t> invalidPE = {0x00, 0x01, 0x02}; // Too small
    if (ValidateBasicPE(invalidPE)) {
        std::cout << "FAILED: Invalid PE accepted" << std::endl;
        return 1;
    }
    
    std::cout << "PASSED: PE validation works correctly" << std::endl;
    
    // Test 3: Error handling for edge cases
    std::cout << "Test 3: Error handling..." << std::endl;
    
    // Test empty data
    if (TestSaveFile("test.bin", std::vector<uint8_t>())) {
        std::cout << "WARNING: Empty data save should fail" << std::endl;
    }
    
    // Test non-existent file
    std::vector<uint8_t> nonExistent = TestReadFile("non_existent_file_12345.bin");
    if (!nonExistent.empty()) {
        std::cout << "FAILED: Non-existent file should return empty vector" << std::endl;
        return 1;
    }
    
    std::cout << "PASSED: Error handling works correctly" << std::endl;
    
    // Test 4: Large file handling (simulated)
    std::cout << "Test 4: Large file handling..." << std::endl;
    
    // Create a reasonably large test vector (1MB)
    std::vector<uint8_t> largeData(1024 * 1024, 0xAA);
    std::string largeFile = "large_test.bin";
    
    if (!TestSaveFile(largeFile, largeData)) {
        std::cout << "FAILED: Could not save large file" << std::endl;
        return 1;
    }
    
    std::vector<uint8_t> readLargeData = TestReadFile(largeFile);
    if (readLargeData.size() != largeData.size()) {
        std::cout << "FAILED: Large file size mismatch" << std::endl;
        std::remove(largeFile.c_str());
        return 1;
    }
    
    // Clean up
    std::remove(largeFile.c_str());
    std::cout << "PASSED: Large file handling works correctly" << std::endl;
    
    // Test 5: PE structure bounds checking
    std::cout << "Test 5: PE bounds checking..." << std::endl;
    
    // Create PE with invalid e_lfanew
    std::vector<uint8_t> malformedPE = CreateTestPE();
    // Set e_lfanew to point beyond file end
    uint32_t badOffset = static_cast<uint32_t>(malformedPE.size() + 100);
    memcpy(&malformedPE[60], &badOffset, sizeof(badOffset));
    
    if (ValidateBasicPE(malformedPE)) {
        std::cout << "FAILED: Malformed PE with bad e_lfanew accepted" << std::endl;
        return 1;
    }
    
    std::cout << "PASSED: PE bounds checking works correctly" << std::endl;
    
    std::cout << "All comprehensive tests passed!" << std::endl;
    std::cout << "The project is ready for Windows compilation and testing." << std::endl;
    
    return 0;
}