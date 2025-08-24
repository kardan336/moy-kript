#pragma once
#include <windows.h>

class AntiDebug {
public:
    // Проверка на отладчик
    static bool IsDebuggerPresent();
    
    // Проверка удаленного отладчика
    static bool IsRemoteDebuggerPresent();
    
    // Простая антидисассемблерная защита
    static void ObfuscateCode();
    
    // Проверка времени выполнения (анти-песочница)
    static bool IsTimingAnomalous();
    
    // Основная функция проверки всех методов
    static bool DetectDebugEnvironment();
    
private:
    // Вспомогательные функции
    static void GenerateJunkCode();
    static bool CheckPEB();
};