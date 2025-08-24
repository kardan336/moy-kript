#include "crypt.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

WinCryptor::WinCryptor() : logFile(L"log.txt") {
    LogMessage(L"WinCryptor инициализирован");
}

WinCryptor::~WinCryptor() {
    LogMessage(L"WinCryptor завершен");
}

bool WinCryptor::EncryptFile(const std::wstring& inputPath, const std::wstring& outputPath, const std::string& key) {
    try {
        LogMessage(L"Начало шифрования файла: " + inputPath);
        
        auto data = ReadFile(inputPath);
        if (data.empty()) {
            lastError = L"Не удалось прочитать входной файл";
            LogMessage(L"Ошибка: " + lastError);
            return false;
        }
        
        auto encrypted = XorData(data, key);
        
        if (!WriteFile(outputPath, encrypted)) {
            lastError = L"Не удалось записать зашифрованный файл";
            LogMessage(L"Ошибка: " + lastError);
            return false;
        }
        
        LogMessage(L"Шифрование завершено успешно: " + outputPath);
        return true;
    }
    catch (const std::exception& e) {
        lastError = L"Исключение при шифровании: " + std::wstring(e.what(), e.what() + strlen(e.what()));
        LogMessage(L"Ошибка: " + lastError);
        return false;
    }
}

bool WinCryptor::DecryptFile(const std::wstring& inputPath, const std::wstring& outputPath, const std::string& key) {
    try {
        LogMessage(L"Начало расшифровки файла: " + inputPath);
        
        auto data = ReadFile(inputPath);
        if (data.empty()) {
            lastError = L"Не удалось прочитать зашифрованный файл";
            LogMessage(L"Ошибка: " + lastError);
            return false;
        }
        
        // XOR симметричен, поэтому расшифровка = повторное шифрование
        auto decrypted = XorData(data, key);
        
        if (!WriteFile(outputPath, decrypted)) {
            lastError = L"Не удалось записать расшифрованный файл";
            LogMessage(L"Ошибка: " + lastError);
            return false;
        }
        
        LogMessage(L"Расшифровка завершена успешно: " + outputPath);
        return true;
    }
    catch (const std::exception& e) {
        lastError = L"Исключение при расшифровке: " + std::wstring(e.what(), e.what() + strlen(e.what()));
        LogMessage(L"Ошибка: " + lastError);
        return false;
    }
}

void WinCryptor::LogMessage(const std::wstring& message) {
    std::wofstream logStream(logFile, std::ios::app);
    if (logStream.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::wstringstream timeStr;
        timeStr << std::put_time(std::localtime(&time_t), L"[%Y-%m-%d %H:%M:%S] ");
        
        logStream << timeStr.str() << message << std::endl;
        logStream.close();
    }
}

std::vector<uint8_t> WinCryptor::ReadFile(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();
    
    return buffer;
}

bool WinCryptor::WriteFile(const std::wstring& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    
    return true;
}

std::vector<uint8_t> WinCryptor::XorData(const std::vector<uint8_t>& data, const std::string& key) {
    std::vector<uint8_t> result = data;
    
    if (key.empty()) {
        return result;
    }
    
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    
    return result;
}