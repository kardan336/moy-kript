@echo off
REM build.bat — автоматическая сборка решения в Visual Studio

REM Имя .sln файла (не меняйте, если не переименовываете sln)
set SOLUTION=WinCryptor.sln

REM Конфигурация сборки
set CONFIG=Release

REM Платформа сборки
set PLATFORM=x64

REM Проверка наличия MSBuild
where MSBuild >nul 2>nul
if errorlevel 1 (
    echo [ERROR] MSBuild не найден! Добавьте путь к MSBuild.exe в переменную среды PATH.
    echo Обычно он находится здесь: "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    exit /b 1
)

REM Сборка решения
echo [INFO] Сборка %SOLUTION% (%CONFIG%|%PLATFORM%)
MSBuild "%SOLUTION%" /p:Configuration=%CONFIG%;Platform=%PLATFORM%

if errorlevel 1 (
    echo [ERROR] Сборка завершилась с ошибками!
    exit /b 1
) else (
    echo [SUCCESS] Сборка завершена успешно.
)
pause
