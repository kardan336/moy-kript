#include "special_crypt_tab.h"
#include "../resource.h"
#include "../randomized_stub.h"
#include <commctrl.h>
#include <fstream>
#include <vector>
#include <string>

static HWND hLog = nullptr;

void LogSpecial(HWND hLog, const std::wstring& msg) {
    int len = GetWindowTextLengthW(hLog);
    SendMessageW(hLog, EM_SETSEL, len, len);
    SendMessageW(hLog, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
    SendMessageW(hLog, EM_SCROLLCARET, 0, 0);
}

std::vector<uint8_t> ReadFileW(const std::wstring& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void SaveFileW(const std::wstring& path, const std::vector<uint8_t>& data) {
    std::ofstream out(path, std::ios::binary);
    out.write((const char*)data.data(), data.size());
}

void DoSpecialCrypt(HWND hDlg) {
    wchar_t src[MAX_PATH] = L"", dst[MAX_PATH] = L"";
    GetDlgItemTextW(hDlg, IDC_SPECIAL_SRC, src, MAX_PATH-1);
    GetDlgItemTextW(hDlg, IDC_SPECIAL_DST, dst, MAX_PATH-1);

    if (!*src || !*dst) {
        LogSpecial(hLog, L"[Ошибка] Укажите оба пути к файлам!\r\n");
        return;
    }
    auto exe = ReadFileW(src);
    if (exe.empty()) {
        LogSpecial(hLog, L"[Ошибка] Не удалось прочитать исходный EXE.\r\n");
        return;
    }
    // Генерируем уникальную оболочку
    std::vector<uint8_t> stub;
    if (!GenerateRandomizedStub(exe, stub)) {
        LogSpecial(hLog, L"[Ошибка] Не удалось создать оболочку.\r\n");
        return;
    }
    SaveFileW(dst, stub);
    LogSpecial(hLog, L"[OK] Особая шифровка завершена!\r\n");
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