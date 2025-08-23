#include "lang.h"
#include <map>

static Lang g_lang = Lang::RU;
static std::map<std::wstring, std::wstring> ru, en;

void SetLang(Lang l) { g_lang = l; }
Lang GetLang() { return g_lang; }

const wchar_t* L(const wchar_t* key) {
    auto& dict = (g_lang == Lang::RU) ? ru : en;
    auto it = dict.find(key);
    return (it != dict.end()) ? it->second.c_str() : key;
}

void LoadLangStrings() {
    ru = {
        {L"SPECIAL_CRYPT", L"Особая шифровка"},
        {L"VT", L"VirusTotal"},
        {L"LANG_SWITCH", L"Сменить язык"},
        {L"OPEN", L"Открыть"},
        {L"SAVE", L"Сохранить как"},
        {L"SUCCESS", L"Успешно!"},
        {L"ERROR", L"Ошибка!"}
    };
    en = {
        {L"SPECIAL_CRYPT", L"Special Crypt"},
        {L"VT", L"VirusTotal"},
        {L"LANG_SWITCH", L"Switch Language"},
        {L"OPEN", L"Open"},
        {L"SAVE", L"Save as"},
        {L"SUCCESS", L"Success!"},
        {L"ERROR", L"Error!"}
    };
}

void ApplyLang(HWND hWnd) {
    SetWindowTextW(GetDlgItem(hWnd, IDC_SPECIAL_BTN_CRYPT), L(L"SPECIAL_CRYPT"));
    SetWindowTextW(GetDlgItem(hWnd, IDC_VT_BTN), L(L"VT"));
    SetWindowTextW(GetDlgItem(hWnd, IDC_LANG_SWITCH), L(L"LANG_SWITCH"));
}