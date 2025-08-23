#include "main_window.h"
#include "special_crypt_tab.h"
#include "../lang.h"
#include <commctrl.h>

static HWND g_hTab, g_hCloudCryptTab;

INT_PTR CALLBACK MainWindowProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        g_hTab = GetDlgItem(hDlg, IDC_TAB);
        g_hCloudCryptTab = GetDlgItem(hDlg, IDD_TAB_CLOUD_CRYPT);
        SetWindowTextW(hDlg, L"WinCryptor");
        ApplyLang(hDlg);
        // Вкладка только одна, можно добавить динамический TabControl, если потребуется больше вкладок
        SetWindowLongPtrW(g_hCloudCryptTab, GWLP_WNDPROC, (LONG_PTR)SpecialCryptTabProc);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LANG_SWITCH:
            SetLang(GetLang() == Lang::RU ? Lang::EN : Lang::RU);
            ApplyLang(hDlg);
            break;
        }
        break;
    }
    return FALSE;
}