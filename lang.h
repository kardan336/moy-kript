#pragma once
#include <string>
#include <windows.h>

enum class Lang { RU, EN };

void SetLang(Lang);
Lang GetLang();

const wchar_t* L(const wchar_t* key);
void LoadLangStrings();
void ApplyLang(HWND hWnd);