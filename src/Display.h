#pragma once
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

class Display {
private:
    LiquidCrystal_I2C& lcd;
    char prevLine0[17];
    char prevLine1[17];

    static const char clearLine[] PROGMEM;

    void printFromProgmem(const char* str) {
        char c;
        while ((c = pgm_read_byte(str++))) {
            lcd.write(c);
        }
    }

    void updateLine(uint8_t row, const char* newLine) {
        char* prevLine = row ? prevLine1 : prevLine0;
        if (strcmp(newLine, prevLine) != 0) {
            lcd.setCursor(0, row);
            printFromProgmem(clearLine);
            lcd.setCursor(0, row);
            lcd.print(newLine);
            strcpy(prevLine, newLine);
        }
    }

public:
    Display(LiquidCrystal_I2C& lcdInstance) : lcd(lcdInstance) {
        memset(prevLine0, 0, sizeof(prevLine0));
        memset(prevLine1, 0, sizeof(prevLine1));
    }

    // Для нового меню
    void showMenuScreen(const char* item, uint8_t current, uint8_t total) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "%s", item);
        snprintf(line1, sizeof(line1), "%d/%d", current, total);
        updateLine(0, line0);
        updateLine(1, line1);
    }

    void showMainScreen(float temperature, bool mixerState, bool coolerState) {
        char line0[17], line1[17];
        
        dtostrf(temperature, 5, 1, line0 + 6);
        memcpy_P(line0, PSTR("Temp: "), 6);
        strcat_P(line0, PSTR(" C"));
        
        strcpy_P(line1, PSTR("Mix:"));
        strcat_P(line1, mixerState ? PSTR("ON ") : PSTR("OFF"));
        strcat_P(line1, PSTR(" Cool:"));
        strcat_P(line1, coolerState ? PSTR("ON") : PSTR("OFF"));

        updateLine(0, line0);
        updateLine(1, line1);
    }

    void showWashingScreen(const char* stage, int remainingTime) {
        char line0[17], line1[17];
        
        strncpy(line0, "Stage: ", 7);
        strncat(line0, stage, 9);
        line0[16] = '\0';
        
        strncpy(line1, "Time: ", 6);
        itoa(remainingTime, line1 + 6, 10);
        strncat(line1, "s", 1);
        line1[16] = '\0';
        
        updateLine(0, line0);
        updateLine(1, line1);
    }

    void showMessage(const char* message) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(message);
        strncpy(prevLine0, message, 16);
        prevLine0[16] = '\0';
        prevLine1[0] = '\0';
    }
};

const char Display::clearLine[] PROGMEM = "                ";