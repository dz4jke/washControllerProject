// Display.h
#pragma once
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

class Display {
private:
    LiquidCrystal_I2C& lcd;    // Ссылка на объект LCD
    char prevLine0[17];        // Предыдущее состояние строки 0
    char prevLine1[17];        // Предыдущее состояние строки 1

    // Строки в памяти программ (PROGMEM)
    static const char clearLine[] PROGMEM;

    // Вывод строки из PROGMEM
    void printFromProgmem(const char* str) {
        char c;
        while ((c = pgm_read_byte(str++))) {
            lcd.write(c);
        }
    }

    // Обновление строки дисплея, если она изменилась
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
    // Конструктор
    Display(LiquidCrystal_I2C& lcdInstance) : lcd(lcdInstance) {
        memset(prevLine0, 0, sizeof(prevLine0));
        memset(prevLine1, 0, sizeof(prevLine1));
    }

    // Отображение главного экрана
    void showMainScreen(float temperature, bool mixerState, bool coolerState) {
    char line0[17], line1[17];
    
    // Формирование строки температуры (правильный порядок!)
    strcpy_P(line0, PSTR("Temp: "));  // Сначала инициализируем строку
    dtostrf(temperature, 5, 1, line0 + 6); // Затем добавляем температуру
    strcat_P(line0, PSTR(" C"));      // Добавляем единицы измерения
    
    // Формирование строки состояния
    strcpy_P(line1, PSTR("Mix:"));
    strcat_P(line1, mixerState ? PSTR("ON ") : PSTR("OFF"));
    strcat_P(line1, PSTR(" Cool:"));
    strcat_P(line1, coolerState ? PSTR("ON") : PSTR("OFF"));

    updateLine(0, line0);
    updateLine(1, line1);
}

    // Отображение экрана мойки
    void showWashingScreen(const char* stage, int remainingTime) {
        char line0[17], line1[17];
        
        strcpy_P(line0, PSTR("Stage: "));
        strncat(line0, stage, 9);
        
        strcpy_P(line1, PSTR("Time: "));
        itoa(remainingTime, line1 + 6, 10);
        strcat_P(line1, PSTR("s"));

        // Для экрана мойки всегда обновляем
        lcd.setCursor(0, 0);
        printFromProgmem(clearLine);
        lcd.setCursor(0, 0);
        lcd.print(line0);
        
        lcd.setCursor(0, 1);
        printFromProgmem(clearLine);
        lcd.setCursor(0, 1);
        lcd.print(line1);
        
        strcpy(prevLine0, line0);
        strcpy(prevLine1, line1);
    }

    // Отображение сообщения
    void showMessage(const char* message) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(message);
        strncpy(prevLine0, message, 16);
        prevLine0[16] = '\0';
        prevLine1[0] = '\0';
    }
};

// Инициализация PROGMEM строк
const char Display::clearLine[] PROGMEM = "                ";