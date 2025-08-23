#include <windows.h>
#include "gui/main_window.h"
#include "lang.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    LoadLangStrings();
    return DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN_DIALOG), NULL, MainWindowProc);
}