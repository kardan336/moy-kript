# WinCryptor

WinCryptor — кроссплатформенное приложение для шифрования/дешифрования данных на C++.

## Возможности
- Простое шифрование/дешифрование строк.
- Поддержка Visual Studio и CMake.
- Легко расширяем для добавления новых алгоритмов.

## Сборка

### Visual Studio
1. Откройте `WinCryptor.sln` или `.vcxproj` в Visual Studio.
2. Постройте проект (Ctrl+Shift+B).

### CMake (Windows/Unix)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Тестирование

```bash
ctest --output-on-failure
```

## Структура проекта

```
WinCryptor/
 ├── WinCryptor.h / WinCryptor.cpp
 ├── main.cpp
 ├── tests/
 │    └── test_wincryptor.cpp
 ├── CMakeLists.txt
 ├── README.md
 ├── .gitignore
 └── LICENSE
```

## Зависимости
- C++17 или новее
- Для тестов — Google Test