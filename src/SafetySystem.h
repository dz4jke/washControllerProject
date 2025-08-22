#pragma once
#include <avr/wdt.h>
#include <string.h>

/*
 * Класс системы безопасности
 * Реализует:
 * - Контроль активности системы
 * - Защиту паролем
 * - Блокировку при неверном вводе
 */
class SafetySystem {
private:
    const unsigned long TIMEOUT = 10000UL;    // Таймаут бездействия (мс)
    const uint8_t MAX_ATTEMPTS = 3;          // Макс. число попыток ввода
    const unsigned long LOCK_TIME = 300000UL; // Время блокировки (мс)
    
    unsigned long lastActivity;    // Время последней активности
    unsigned long lockUntil;       // Время до разблокировки
    uint8_t wrongAttempts;        // Количество неверных попыток

    // Пароль в PROGMEM
    static const char password[] PROGMEM;

public:
    /*
     * Конструктор
     */
    SafetySystem() : 
        lastActivity(millis()),
        lockUntil(0),
        wrongAttempts(0) 
    {}

    /*
     * Проверка таймаута бездействия
     * Вызывает перезагрузку при превышении таймаута
     */
    void checkActivity() {
        if((millis() - lastActivity) > TIMEOUT) {
            wdt_enable(WDTO_15MS); // Активируем watchdog
            while(1);              // Бесконечный цикл для перезагрузки
        }
    }

    /*
     * Обновление времени последней активности
     */
    void updateActivity() {
        lastActivity = millis();
    }

    /*
     * Проверка пароля
     * input - введенный пароль
     * Возвращает true при совпадении
     */
    bool checkPassword(const char* input) {
        if(isLocked()) return false;
        
        // Сравнение с паролем из PROGMEM
        if(strcmp_P(input, password) == 0) {
            wrongAttempts = 0;
            return true;
        }
        
        // Увеличение счетчика неверных попыток
        if(++wrongAttempts >= MAX_ATTEMPTS) {
            lockUntil = millis() + LOCK_TIME;
        }
        return false;
    }

    /*
     * Проверка блокировки системы
     * Возвращает true если система заблокирована
     */
    bool isLocked() const {
        return millis() < lockUntil;
    }

    /*
     * Получение оставшегося времени блокировки
     * Возвращает время в секундах
     */
    uint16_t getLockRemaining() const {
        if(!isLocked()) return 0;
        unsigned long remaining = lockUntil - millis();
        return (remaining + 999) / 1000; // Округление вверх
    }
};

// Пароль в программной памяти
const char SafetySystem::password[] PROGMEM = "1234";