#pragma once
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h> // Для работы с PROGMEM
#include <string.h>       // Для strcmp, strcpy, strncpy, memset
#include <stdio.h>        // Для snprintf
#include <stdlib.h>       // Для dtostrf, itoa

class Display {
private:
    LiquidCrystal_I2C& lcd;
    char prevLine0[17]; // Буфер для предыдущего состояния первой строки
    char prevLine1[17]; // Буфер для предыдущего состояния второй строки

    // Строка для очистки строки дисплея, хранится в PROGMEM
    static const char clearLine[] PROGMEM;

    /*
     * Вспомогательная функция для печати строки из PROGMEM
     * (используется для внутренней логики, чтобы избежать повторного считывания)
     */
    void printFromProgmem(const char* str) {
        char c;
        while ((c = pgm_read_byte(str++))) {
            lcd.write(c);
        }
    }

    /*
     * Обновляет строку на LCD, только если содержимое изменилось
     */
    void updateLine(uint8_t row, const char* newLine) {
        char* prevLine = (row == 0) ? prevLine0 : prevLine1;

        if (strcmp(newLine, prevLine) != 0) { // Сравниваем текущую строку с предыдущей
            lcd.setCursor(0, row);
            printFromProgmem(clearLine); // Очищаем строку, читая из PROGMEM
            lcd.setCursor(0, row);
            lcd.print(newLine);
            strcpy(prevLine, newLine); // Сохраняем новую строку как предыдущую
        }
    }

public:
    /*
     * Конструктор
     * lcdInstance - ссылка на объект LiquidCrystal_I2C
     */
    Display(LiquidCrystal_I2C& lcdInstance) : lcd(lcdInstance) {
        // Инициализация буферов нулями
        memset(prevLine0, 0, sizeof(prevLine0));
        memset(prevLine1, 0, sizeof(prevLine1));
    }

    /*
     * Отображает экран меню
     * item - текст текущего пункта меню
     * current - номер текущего пункта (1-based)
     * total - общее количество пунктов
     */
    void showMenuScreen(const char* item, uint8_t current, uint8_t total) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "%s", item);
        snprintf(line1, sizeof(line1), "%d/%d", current, total);
        updateLine(0, line0);
        updateLine(1, line1);
    }

    /*
     * Отображает главный экран с температурой, состоянием миксера и компрессора
     */
    void showMainScreen(float temperature, bool mixerState, bool coolerState) {
        char line0[17]; // Достаточно для "Temp: -XX.X C"
        char line1[17]; // Достаточно для "Mix:ON Cool:OFF"
        
        // Форматирование первой строки: "Temp: XX.X C"
        snprintf(line0, sizeof(line0), "Temp: %5.1f C", temperature);
        
        // Форматирование второй строки: "Mix: ON/OFF Cool: ON/OFF"
        snprintf(line1, sizeof(line1), "Mix:%s Cool:%s", 
                 mixerState ? "ON " : "OFF", // Добавлен пробел для выравнивания
                 coolerState ? "ON" : "OFF");

        updateLine(0, line0);
        updateLine(1, line1);
    }

    /*
     * Отображает экран процесса мойки
     */
    void showWashingScreen(const char* stage, int remainingTime) {
        char line0[17], line1[17];
        
        // Форматирование первой строки: "Stage: <Stage Name>"
        snprintf(line0, sizeof(line0), "Stage: %s", stage);
        
        // Форматирование второй строки: "Time: XXs"
        snprintf(line1, sizeof(line1), "Time: %ds", remainingTime);
        
        updateLine(0, line0);
        updateLine(1, line1);
    }

    /*
     * Отображает временное сообщение на дисплее (очищает обе строки)
     */
    void showMessage(const char* message) {
        lcd.clear(); // Очищаем весь дисплей
        lcd.setCursor(0, 0);
        lcd.print(message);
        // Обновляем prevLine0 и очищаем prevLine1 для корректной работы updateLine
        strncpy(prevLine0, message, 16);
        prevLine0[16] = '\0';
        prevLine1[0] = '\0'; // Гарантируем, что вторая строка будет обновлена следующим вызовом updateLine
    }
};

// Строка для очистки дисплея, хранится в PROGMEM
const char Display::clearLine[] PROGMEM = "                "; // 16 пробелов
