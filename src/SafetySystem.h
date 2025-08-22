#pragma once
#include <avr/wdt.h> // Для работы с Watchdog Timer
#include <string.h>  // Для strcmp_P
#include <Arduino.h> // Для millis()

/*
 * Класс системы безопасности
 * Реализует:
 * - Контроль активности системы (через Watchdog)
 * - Проверку пароля с блокировкой при неверном вводе
 */
class SafetySystem {
private:
    const unsigned long TIMEOUT = 10000UL;      // Таймаут бездействия для Watchdog (мс)
    const uint8_t MAX_ATTEMPTS = 3;             // Макс. число попыток ввода пароля
    const unsigned long LOCK_TIME = 300000UL;   // Время блокировки при неверном вводе (мс)
    
    unsigned long lastActivity;      // Время последней активности (для Watchdog)
    unsigned long lockUntil;         // Время, до которого система заблокирована
    uint8_t wrongAttempts;           // Количество неверных попыток ввода пароля

    // Пароль хранится в программной памяти (PROGMEM)
    // ВНИМАНИЕ: Статический пароль в коде - небезопасное решение. 
    // Для реальных систем используйте хранение в EEPROM или более сложные методы.
    static const char password[] PROGMEM;

public:
    /*
     * Конструктор
     */
    SafetySystem() : 
        lastActivity(millis()), // Инициализация времени последней активности текущим временем
        lockUntil(0),           // Изначально система не заблокирована
        wrongAttempts(0)        // Изначально 0 неверных попыток
    {}

    /*
     * Проверка таймаута бездействия
     * Если система не обновляла активность в течение TIMEOUT, вызывается перезагрузка.
     */
    void checkActivity() {
        if((millis() - lastActivity) > TIMEOUT) {
            // Активируем Watchdog на минимальное время
            wdt_enable(WDTO_15MS); 
            // Входим в бесконечный цикл, чтобы Watchdog сработал и перезагрузил устройство
            while(1); 
        }
    }

    /*
     * Обновление времени последней активности
     * Сбрасывает таймер Watchdog. Должен вызываться регулярно.
     */
    void updateActivity() {
        lastActivity = millis();
    }

    /*
     * Проверка пароля
     * input - введенный пользователем пароль
     * Возвращает true при совпадении паролей, false в противном случае.
     */
    bool checkPassword(const char* input) {
        if(isLocked()) return false; // Если система заблокирована, пароль не принимается
        
        // Сравнение введенного пароля с паролем из PROGMEM
        if(strcmp_P(input, password) == 0) {
            wrongAttempts = 0; // Сбрасываем счетчик неверных попыток при успешном входе
            return true;
        }
        
        // Увеличиваем счетчик неверных попыток
        if(++wrongAttempts >= MAX_ATTEMPTS) {
            lockUntil = millis() + LOCK_TIME; // Блокируем систему
        }
        return false;
    }

    /*
     * Проверка, заблокирована ли система
     * Возвращает true, если система находится в состоянии блокировки
     */
    bool isLocked() const {
        return millis() < lockUntil; // Система заблокирована, если текущее время меньше lockUntil
    }

    /*
     * Получение оставшегося времени до разблокировки системы
     * Возвращает время в секундах.
     */
    uint16_t getLockRemaining() const {
        if(!isLocked()) return 0; // Если не заблокирована, возвращаем 0
        unsigned long remaining = lockUntil - millis();
        return (remaining + 999) / 1000; // Округляем вверх до ближайшей секунды
    }
};

// Пароль в программной памяти (PROGMEM)
const char SafetySystem::password[] PROGMEM = "1234";
