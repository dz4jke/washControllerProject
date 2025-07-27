#pragma once
#include <avr/wdt.h>

class SafetySystem {
private:
    unsigned long lastActivity = 0;
    const unsigned long TIMEOUT = 10000;
    uint8_t wrongAttempts = 0;
    const uint8_t MAX_ATTEMPTS = 3;
    unsigned long lockUntil = 0;
    const unsigned long LOCK_TIME = 300000;

public:
    void checkActivity() {
        if(millis() - lastActivity > TIMEOUT) {
            wdt_enable(WDTO_15MS);
            while(1);
        }
    }

    void updateActivity() {
        lastActivity = millis();
    }

    bool checkPassword(const char* input) {
        if(isLocked()) return false;
        
        if(strcmp(input, "1234") == 0) {
            wrongAttempts = 0;
            return true;
        }
        
        if(++wrongAttempts >= MAX_ATTEMPTS) {
            lockUntil = millis() + LOCK_TIME;
        }
        return false;
    }

    bool isLocked() const {
        return millis() < lockUntil;
    }

    uint16_t getLockRemaining() const {
        return isLocked() ? (lockUntil - millis()) / 1000 : 0;
    }
};