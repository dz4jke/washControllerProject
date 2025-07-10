#pragma once  // Защита от множественного включения заголовочного файла
#include <GyverButton.h>  // Подключение библиотеки для работы с кнопками

class ButtonHandler {
private:
    // Объекты кнопок из библиотеки GyverButton
    GButton btnUp;    // Кнопка "Вверх"
    GButton btnDown;  // Кнопка "Вниз"
    GButton btnSet;   // Кнопка "Установка"
    GButton btnEsc;   // Кнопка "Отмена"
    bool longPress = false;  // Флаг длительного нажатия

public:
    // Конструктор класса, принимающий номера пинов кнопок
    ButtonHandler(uint8_t upPin, uint8_t downPin, uint8_t setPin, uint8_t escPin)
        : btnUp(upPin), btnDown(downPin), btnSet(setPin), btnEsc(escPin) {
        
        // Настройка режима работы кнопок - автоматический опрос
        btnUp.setTickMode(AUTO);
        btnDown.setTickMode(AUTO);
        btnSet.setTickMode(AUTO);
        btnEsc.setTickMode(AUTO);
    }

    // Метод обновления состояния кнопок
    void update() {
        btnUp.tick();    // Обновление состояния кнопки "Вверх"
        btnDown.tick();  // Обновление состояния кнопки "Вниз"
        btnSet.tick();   // Обновление состояния кнопки "Установка"
        btnEsc.tick();   // Обновление состояния кнопки "Отмена"

        // Проверка длительного нажатия на кнопку "Установка"
        if(btnSet.isHolded()) {
            longPress = true;  // Установка флага длительного нажатия
        }
    }

    // Методы проверки нажатия кнопок
    bool isUpPressed() { return btnUp.isClick(); }    // Было ли нажатие "Вверх"
    bool isDownPressed() { return btnDown.isClick(); } // Было ли нажатие "Вниз"
    bool isSetPressed() { return btnSet.isClick() || longPress; } // Нажатие или удержание "Установка"
    bool isEscPressed() { return btnEsc.isClick(); }  // Было ли нажатие "Отмена"
    
    // Сброс флага длительного нажатия
    void resetLongPress() { 
        longPress = false; 
    }
};