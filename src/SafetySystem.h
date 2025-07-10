#pragma once
#include <avr/wdt.h>  // Подключение watchdog таймера

class SafetySystem {
private:
    unsigned long lastActivity = 0;  // Время последней активности
    const unsigned long TIMEOUT = 10000;  // Таймаут бездействия (10 сек)
    uint8_t wrongAttempts = 0;  // Количество неудачных попыток ввода пароля
    const uint8_t MAX_ATTEMPTS = 3;  // Максимальное количество попыток
    unsigned long lockUntil = 0;  // Время до разблокировки
    const unsigned long LOCK_TIME = 300000;  // Время блокировки (5 мин)

public:
    // Проверка активности системы
    void checkActivity() {
        if(millis() - lastActivity > TIMEOUT) {  // Если таймаут превышен
            wdt_enable(WDTO_15MS);  // Активация watchdog с минимальным таймаутом
            while(1);  // Бесконечный цикл - приведет к перезагрузке
        }
    }

    // Обновление времени последней активности
    void updateActivity() {
        lastActivity = millis();
    }

    // Проверка пароля
    bool checkPassword(const char* input) {
        if(isLocked()) return false;  // Если система заблокирована
        
        if(strcmp(input, "1234") == 0) {  // Сравнение с паролем "1234"
            wrongAttempts = 0;  // Сброс счетчика попыток
            return true;
        }
        
        // Увеличение счетчика неудачных попыток
        if(++wrongAttempts >= MAX_ATTEMPTS) {
            lockUntil = millis() + LOCK_TIME;  // Установка времени блокировки
        }
        return false;
    }

    // Проверка, заблокирована ли система
    bool isLocked() const {
        return millis() < lockUntil;
    }

    // Получение оставшегося времени блокировки
    uint16_t getLockRemaining() const {
        return isLocked() ? (lockUntil - millis()) / 1000 : 0;
    }
};