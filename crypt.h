#pragma once
#include <vector>
#include <string>
#include <fstream>

class WinCryptor {
public:
    WinCryptor();
    ~WinCryptor();
    
    // Шифрование файла с использованием XOR
    bool EncryptFile(const std::wstring& inputPath, const std::wstring& outputPath, const std::string& key);
    
    // Расшифровка файла (обратный XOR)
    bool DecryptFile(const std::wstring& inputPath, const std::wstring& outputPath, const std::string& key);
    
    // Получить последнюю ошибку
    std::wstring GetLastError() const { return lastError; }
    
    // Логирование в файл
    void LogMessage(const std::wstring& message);
    
private:
    std::wstring lastError;
    std::wstring logFile;
    
    // Чтение файла в буфер
    std::vector<uint8_t> ReadFile(const std::wstring& path);
    
    // Запись буфера в файл
    bool WriteFile(const std::wstring& path, const std::vector<uint8_t>& data);
    
    // XOR операция
    std::vector<uint8_t> XorData(const std::vector<uint8_t>& data, const std::string& key);
};