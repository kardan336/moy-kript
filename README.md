# WinCryptor

Пример консольного приложения C++ для Windows (Visual Studio 2019/2022, x64, C++17).

## Сборка

1. Откройте `WinCryptor.sln` в Visual Studio.
2. Выберите платформу `x64` и конфигурацию `Debug` или `Release`.
3. Постройте и запустите проект (`Ctrl+F5`).

## Структура

- `WinCryptor/` — исходники проекта (main.cpp, WinCryptor.cpp, WinCryptor.h, WinCryptor.vcxproj)
- `.gitignore` — исключает временные/служебные файлы
- `README.md` — инструкция
- `WinCryptor.sln` — файл решения Visual Studio

## Пример вывода

```
Original: Hello, WinCryptor!
Encrypted: !rotpyrcniW ,olleH
Decrypted: Hello, WinCryptor!
```
