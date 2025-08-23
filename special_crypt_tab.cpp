#include "special_crypt_tab.h"
#include "../resource.h"
#include "../randomized_stub.h"
#include <commctrl.h>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>

static HWND hLog = nullptr;

void LogSpecial(HWND hLog, const std::wstring& msg) {
    int len = GetWindowTextLengthW(hLog);
    SendMessageW(hLog, EM_SETSEL, len, len);
    SendMessageW(hLog, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
    SendMessageW(hLog, EM_SCROLLCARET, 0, 0);
}

std::vector<uint8_t> ReadFileW(const std::wstring& path) {
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

bool SaveFileW(const std::wstring& path, const std::vector<uint8_t>& data) {
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

void DoSpecialCrypt(HWND hDlg) {
    wchar_t src[MAX_PATH] = L"", dst[MAX_PATH] = L"";
    GetDlgItemTextW(hDlg, IDC_SPECIAL_SRC, src, MAX_PATH-1);
    GetDlgItemTextW(hDlg, IDC_SPECIAL_DST, dst, MAX_PATH-1);

    // Null-terminate for safety
    src[MAX_PATH-1] = L'\0';
    dst[MAX_PATH-1] = L'\0';

    if (!*src || !*dst) {
        LogSpecial(hLog, L"[Ошибка] Укажите оба пути к файлам!\r\n");
        return;
    }
    
    LogSpecial(hLog, L"[Инфо] Начало обработки файла...\r\n");
    
    auto exe = ReadFileW(src);
    if (exe.empty()) {
        LogSpecial(hLog, L"[Ошибка] Не удалось прочитать исходный EXE или файл пустой.\r\n");
        return;
    }
    
    // Basic PE validation
    if (exe.size() < sizeof(IMAGE_DOS_HEADER)) {
        LogSpecial(hLog, L"[Ошибка] Файл слишком мал для PE.\r\n");
        return;
    }
    
    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(exe.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        LogSpecial(hLog, L"[Ошибка] Файл не является валидным PE.\r\n");
        return;
    }
    
    LogSpecial(hLog, L"[Инфо] Генерация уникальной оболочки...\r\n");
    
    // Генерируем уникальную оболочку
    std::vector<uint8_t> stub;
    if (!GenerateRandomizedStub(exe, stub)) {
        LogSpecial(hLog, L"[Ошибка] Не удалось создать оболочку. Проверьте наличие MinGW g++.\r\n");
        return;
    }
    
    if (!SaveFileW(dst, stub)) {
        LogSpecial(hLog, L"[Ошибка] Не удалось сохранить результат.\r\n");
        return;
    }
    
    LogSpecial(hLog, L"[OK] Особая шифровка завершена! Размер: ");
    LogSpecial(hLog, std::to_wstring(stub.size()));
    LogSpecial(hLog, L" байт\r\n");
}

INT_PTR CALLBACK SpecialCryptTabProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        hLog = GetDlgItem(hDlg, IDC_SPECIAL_LOG);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SPECIAL_BTN_SRC: {
            wchar_t buf[MAX_PATH] = L"";
            OPENFILENAMEW ofn = { sizeof(ofn) };
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"EXE Files (*.exe)\0*.exe\0All Files\0*.*\0";
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileNameW(&ofn))
                SetWindowTextW(GetDlgItem(hDlg, IDC_SPECIAL_SRC), buf);
            break;
        }
        case IDC_SPECIAL_BTN_DST: {
            wchar_t buf[MAX_PATH] = L"";
            OPENFILENAMEW ofn = { sizeof(ofn) };
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"EXE Files (*.exe)\0*.exe\0All Files\0*.*\0";
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileNameW(&ofn))
                SetWindowTextW(GetDlgItem(hDlg, IDC_SPECIAL_DST), buf);
            break;
        }
        case IDC_SPECIAL_BTN_CRYPT:
            DoSpecialCrypt(hDlg);
            break;
        }
        break;
    }
    return FALSE;
}