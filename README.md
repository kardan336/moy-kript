# WinCryptor

**WinCryptor** — продвинутый криптор для Windows-приложений (EXE/PE), создающий уникальную оболочку stub с junk-кодом, современными техниками анти-анализа и реальными реализациями RunPE/Process Hollowing для каждого зашифрованного файла.

---

## Возможности

- ✅ **Реальные реализации инъекции PE**: RunPE и Process Hollowing для Windows x86/x64
- ✅ **Продвинутая анти-отладка**: PEB BeingDebugged, NtGlobalFlag, heap flags, NtQueryInformationProcess
- ✅ **Современная анти-VM**: CPUID hypervisor bit, MAC-адреса, файлы драйверов VM, реестр
- ✅ **Анти-сандбокс**: проверка процессов, uptime, RAM, пользователей, окон анализа
- ✅ **Генерация уникального junk-кода**: рандомные вычисления, циклы, системные вызовы
- ✅ **Автоматическая рандомизация**: ключи шифрования, build-time, строки, методы инъекции
- ✅ **Криптование XOR** с уникальными ключами для каждого stub'а
- ✅ **Локализация интерфейса** (русский/английский, переключение на лету)
- ✅ **Интеграция с VirusTotal** (через браузер)

---

## Как собрать и запустить

### Требования

1. **Windows 10/11 x64** (для финальной сборки и тестирования)
2. **Visual Studio 2019/2022** с Desktop Development with C++
3. **MinGW-w64 (g++)** в PATH для сборки stub'ов
   - Скачать: https://www.mingw-w64.org/downloads/
   - Или через MSYS2: `pacman -S mingw-w64-x86_64-gcc`
4. **Git** для клонирования репозитория

### Сборка проекта

1. **Клонируйте репозиторий:**
   ```bash
   git clone https://github.com/kardan336/moy-kript.git
   cd moy-kript
   ```

2. **Проверьте MinGW-w64:**
   ```cmd
   g++ --version
   ```
   Должна быть версия MinGW-w64, поддерживающая `-mwindows`

3. **Откройте в Visual Studio:**
   - Откройте файл `WinCryptor.sln`
   - Выберите конфигурацию Release x64 (рекомендуется)
   - Build → Rebuild Solution

4. **Запустите WinCryptor.exe**

### Использование

1. **Выберите EXE-файл** для шифрования
2. **Выберите путь для сохранения** зашифрованного stub'а
3. **Нажмите "Особая шифровка"** — будет создан уникальный stub
4. **Результат:** исполняемый файл с встроенными анти-анализом и инъекцией

### Дополнительные инструменты (опционально)

Для расширения функциональности поместите следующие инструменты в папку `tools/`:

- **ConfuserEx** - дополнительная обфускация .NET
- **Enigma Protector** - упаковка и защита
- **LIEF** или **pe-parse** - для продвинутого анализа PE

---

## Технические детали

### Реализованные техники инъекции

**RunPE:**
- Создание suspended процесса
- Unmapping оригинального образа через NtUnmapViewOfSection
- Аллокация памяти под новый PE
- Копирование заголовков и секций
- Обновление PEB и контекста потока
- Resume выполнения

**Process Hollowing:**
- Создание suspended процесса-цели (svchost.exe)
- Получение PEB через NtQueryInformationProcess
- Замена образа в памяти процесса
- Перенаправление точки входа
- Resume выполнения

### Техники анти-анализа

**Анти-отладка:**
- `IsDebuggerPresent()` и `CheckRemoteDebuggerPresent()`
- PEB BeingDebugged flag (x86/x64)
- NtGlobalFlag проверка (0x70)
- Heap flags анализ
- `NtQueryInformationProcess(ProcessDebugPort)`
- `OutputDebugString` timing
- Invalid handle exception (`CloseHandle(0xDEADBEEF)`)

**Анти-VM:**
- CPUID hypervisor bit detection
- MAC-адреса VM (VMware: 00:0C:29, 00:1C:14, 00:50:56; VirtualBox: 08:00:27)
- Файлы драйверов VM
- Процессы VM
- Реестр VM services
- Computer name проверка

**Анти-сандбокс:**
- Проверка uptime (< 10 минут)
- RAM < 2GB
- Отсутствие мыши
- Анализ процессов (cuckoo, sandbox, analyst, malware)
- Пользователи sandbox
- Окна анализа (Process Explorer, WinDbg, etc.)
- Движение мыши

### Генерация stub'а

Каждый запуск создает уникальный stub с:
- **Рандомным XOR ключом** (16 символов)
- **Уникальным junk-кодом** в 6 блоках
- **Случайным набором** анти-анализ проверок
- **Build timestamp** и случайными строками
- **Автовыбором метода** инъекции (RunPE/Hollowing)

---

## Тестирование

### Запуск тестов

```bash
# Тест функций генерации (Linux/Windows)
cd tests
g++ -std=c++17 test_functions.cpp -o test_functions
./test_functions

# Полный тест stub generation (только Windows с MinGW)
g++ -std=c++17 test_stub_generation.cpp ../randomized_stub.cpp -o test_stub_generation
./test_stub_generation.exe
```

### Проверка на Windows

1. **Сгенерируйте stub** с тестовым PE
2. **Запустите в изолированной VM** для проверки анти-VM
3. **Тестируйте в отладчике** для проверки анти-debug
4. **Проверьте в различных sandbox** средах

---

## Структура проекта

```
moy-kript/
├── WinCryptor.sln              # Visual Studio solution
├── WinCryptor.vcxproj          # Проект Visual Studio
├── main.cpp                    # Точка входа приложения
├── main_window.cpp             # Главное окно GUI
├── special_crypt_tab.cpp       # Вкладка шифрования
├── lang.cpp                    # Локализация
├── randomized_stub.cpp         # Генератор уникальных stub'ов
├── stub_template.cpp           # Шаблон для генерации
├── runpe.cpp                   # Реализация RunPE
├── process_hollowing.cpp       # Реализация Process Hollowing
├── pe_ext.cpp                  # Работа с PE файлами
├── av_evasion.cpp             # Evasion техники
├── thirdparty_integration.cpp  # Интеграция внешних tools
├── tests/                      # Unit тесты
│   ├── test_functions.cpp      # Тест функций генерации
│   └── test_stub_generation.cpp # Тест создания stub'а
├── tools/                      # Внешние инструменты (опционально)
└── README.md                   # Этот файл
```

---

## Безопасность и легальность

⚠️ **ВНИМАНИЕ:** Этот проект предназначен **ТОЛЬКО** для:
- Образовательных целей
- Исследования информационной безопасности
- Тестирования антивирусных решений
- Легального пентестинга

**ЗАПРЕЩЕНО** использование для:
- Создания вредоносного ПО
- Нарушения законодательства
- Атак на системы без разрешения

---

## Известные ограничения

1. **Работа только на Windows** (stub generation требует Windows API)
2. **Требует MinGW-w64** для компиляции stub'ов
3. **Некоторые антивирусы** могут детектировать техники инъекции
4. **DEP/ASLR** могут влиять на успешность инъекции

---

## Устранение неполадок

### Ошибка компиляции stub'а
```
g++: command not found
```
**Решение:** Установите MinGW-w64 и добавьте в PATH

### Ошибка "ntdll.dll not found"
**Решение:** Запускайте только на Windows

### Антивирус блокирует файл
**Решение:** Добавьте папку проекта в исключения антивируса

### Stub не запускается
**Решение:** Проверьте:
- Валидность исходного PE файла
- Права на запись в папку назначения
- Версию MinGW-w64 (должна поддерживать Windows headers)

---

## Contributing

1. Fork репозитория
2. Создайте feature branch
3. Тестируйте изменения
4. Создайте Pull Request

---

## Лицензия

Проект предоставляется "как есть" для образовательных целей. Используйте на свой страх и риск с соблюдением применимого законодательства.