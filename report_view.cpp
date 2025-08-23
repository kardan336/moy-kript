#include "report_view.h"
#include <commctrl.h>

void ShowReportView(HWND hWnd, const std::wstring& text) {
    MessageBoxW(hWnd, text.c_str(), L"Report", MB_OK);
}