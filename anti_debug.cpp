#include "anti_debug.h"
#include <winternl.h>
#include <iostream>

bool AntiDebug::IsDebuggerPresent() {
    return ::IsDebuggerPresent();
}

bool AntiDebug::IsRemoteDebuggerPresent() {
    BOOL isRemoteDebugger = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isRemoteDebugger);
    return isRemoteDebugger;
}

void AntiDebug::ObfuscateCode() {
    // Простая антидисассемблерная защита - добавляем мусорный код
    GenerateJunkCode();
    
    // Изменяем порядок выполнения
    volatile int dummy = 0;
    for (int i = 0; i < 1000; ++i) {
        dummy += i * 37;
        dummy ^= 0xDEADBEEF;
    }
}

bool AntiDebug::IsTimingAnomalous() {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    // Выполняем простую операцию
    volatile int temp = 0;
    for (int i = 0; i < 100000; ++i) {
        temp += i;
    }
    
    QueryPerformanceCounter(&end);
    
    // Если операция выполнилась слишком быстро, возможно это песочница
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    return elapsed < 0.001; // Менее 1 мс подозрительно
}

bool AntiDebug::DetectDebugEnvironment() {
    if (IsDebuggerPresent()) {
        return true;
    }
    
    if (IsRemoteDebuggerPresent()) {
        return true;
    }
    
    if (CheckPEB()) {
        return true;
    }
    
    if (IsTimingAnomalous()) {
        return true;
    }
    
    return false;
}

void AntiDebug::GenerateJunkCode() {
    // Генерируем мусорный код для затруднения анализа
    volatile int junk1 = GetTickCount();
    volatile int junk2 = rand();
    volatile int junk3 = junk1 ^ junk2;
    
    // Множественные условные переходы
    if (junk1 > junk2) {
        junk3 += 0x12345678;
    } else {
        junk3 -= 0x87654321;
    }
    
    // Ложные системные вызовы
    SetLastError(junk3);
    GetLastError();
}

bool AntiDebug::CheckPEB() {
    // Проверка PEB на флаг BeingDebugged
    PEB* peb = (PEB*)__readfsdword(0x30);
    if (peb && peb->BeingDebugged) {
        return true;
    }
    
    return false;
}