#pragma once
#include <LiquidCrystal_I2C.h>  // Подключение библиотеки LCD дисплея

class Display {
private:
    LiquidCrystal_I2C* lcd;  // Указатель на объект дисплея
    char currentLine0[17] = {0};  // Текущее содержимое первой строки
    char currentLine1[17] = {0};  // Текущее содержимое второй строки

    // Проверка изменения строки
    bool isLineChanged(const char* newLine, const char* currentLine) {
        return strcmp(newLine, currentLine) != 0;  // Сравнение строк
    }

public:
    // Конструктор, принимающий указатель на LCD
    Display(LiquidCrystal_I2C* lcd) : lcd(lcd) {}

    // Отображение главного экрана
    void showMainScreen(const char* mode, float temp, bool mixerState) {
        char newLine0[17], newLine1[17];  // Буферы для новых строк
Serial.print("Temp value: "); Serial.println(temp);

        // Форматирование строк
        snprintf(newLine0, 16, "%-16s", mode);  // Режим работы (выравнивание по левому краю)
        snprintf(newLine1, 16, "T:%3.1fC M:%s", (double)temp, mixerState ? "ON" : "OFF");

        // Обновление первой строки, если изменилась
        if(isLineChanged(newLine0, currentLine0)) {
            lcd->setCursor(0, 0);  // Установка курсора
            lcd->print(newLine0);   // Вывод строки
            strcpy(currentLine0, newLine0);  // Сохранение текущего состояния
        }

        // Обновление второй строки, если изменилась
        if(isLineChanged(newLine1, currentLine1)) {
            lcd->setCursor(0, 1);
            lcd->print(newLine1);
            strcpy(currentLine1, newLine1);
        }
    }

    // Отображение экрана мойки
    void showWashingScreen(const char* stage, int remainingTime) {
        char newLine0[17], newLine1[17];

        snprintf(newLine0, 16, "%-16s", stage);  // Название этапа
        snprintf(newLine1, 16, "Left: %3d sec", remainingTime);  // Оставшееся время

        // Обновление строк, если изменились
        if(isLineChanged(newLine0, currentLine0)) {
            lcd->setCursor(0, 0);
            lcd->print(newLine0);
            strcpy(currentLine0, newLine0);
        }

        if(isLineChanged(newLine1, currentLine1)) {
            lcd->setCursor(0, 1);
            lcd->print(newLine1);
            strcpy(currentLine1, newLine1);
        }
    }

    // Отображение произвольного сообщения
    void showMessage(const char* msg) {
        lcd->clear();  // Очистка дисплея
        lcd->setCursor(0, 0);  // Курсор в начало
        lcd->print(msg);  // Вывод сообщения
        // Сброс текущих строк
        memset(currentLine0, 0, 17);
        memset(currentLine1, 0, 17);
        strncpy(currentLine0, msg, 16);  // Сохранение сообщения
    }
};