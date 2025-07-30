// ButtonMenuHandler.h
#pragma once
#include <GyverButton.h>
#include "Display.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"

class ButtonMenuHandler
{
private:
    GButton btnUp;   // Кнопка "Вверх"
    GButton btnDown; // Кнопка "Вниз"
    GButton btnSet;  // Кнопка "Выбор"
    GButton btnEsc;  // Кнопка "Отмена"

    Display &display;          // Ссылка на дисплей
    CoolerController &cooler;  // Ссылка на контроллер охлаждения
    MixerController &mixer;    // Ссылка на контроллер миксера
    WashingController &washer; // Ссылка на контроллер мойки

    // Состояния меню
    bool menuActive = false;
    uint8_t currentMenu = 0;
    uint8_t currentItem = 0;
    bool editingValue = false;
    const float editStep = 0.5f; // Шаг изменения значений

    // Пункты меню в PROGMEM
    static const char menuItems[4][16] PROGMEM;

    // Отображение текущего меню
    void showCurrentMenu()
    {
        char buf[16];
        // Копируем 16 байт из PROGMEM в RAM
        memcpy_P(buf, &menuItems[currentMenu][0], 16);
        buf[15] = '\0'; // Обязательно добавляем нуль-терминатор
        display.showMessage(buf);
    }

    // Отображение текущего значения
    void showCurrentValue()
    {
        char buf[21];    // Буфер для итоговой строки (LCD 20x4, 20 символов + '\0')
        char valStr[10]; // Буфер для форматирования чисел (целых или float)
        memset(buf, 0, sizeof(buf));
        memset(valStr, 0, sizeof(valStr));

        switch (currentMenu)
        {
        case 0: // Настройки охлаждения
            switch (currentItem)
            {
            case 0:                                                     // Температура
                dtostrf(cooler.getSettings().targetTemp, 4, 1, valStr); // ширина 4, 1 знак после запятой
                snprintf(buf, sizeof(buf), "Target: %s C", valStr);
                break;
            case 1: // Гистерезис
                dtostrf(cooler.getSettings().hysteresis, 4, 1, valStr);
                snprintf(buf, sizeof(buf), "Hysteresis: %s", valStr);
                break;
            case 2: // Интервал
                itoa(cooler.getSettings().minInterval, valStr, 10);
                snprintf(buf, sizeof(buf), "Min interval: %s", valStr);
                break;
            }
            break;

        case 1: // Настройки миксера
            switch (currentItem)
            {
            case 0:
                itoa(mixer.getSettings().mode, valStr, 10);
                snprintf(buf, sizeof(buf), "Mode: %s", valStr);
                break;
            case 1:
                itoa(mixer.getSettings().workTime, valStr, 10);
                snprintf(buf, sizeof(buf), "Work time: %s", valStr);
                break;
            case 2:
                itoa(mixer.getSettings().idleTime, valStr, 10);
                snprintf(buf, sizeof(buf), "Idle time: %s", valStr);
                break;
            }
            break;

        case 2: // Настройки мойки
            if (currentItem < 5)
            {
                itoa(washer.getSettings().stageTimes[currentItem], valStr, 10);
                snprintf(buf, sizeof(buf), "%s sec", valStr);
            }
            break;
        }
        display.showMessage(buf); // Отображение строки на LCD
    }
    // Редактирование текущего значения
    void editCurrentSetting()
    {
        if (!editingValue)
        {
            editingValue = true;
            static int currentItem = 0;
            showCurrentValue();
        }

        if (btnUp.isClick())
            adjustValue(editStep);
        if (btnDown.isClick())
            adjustValue(-editStep);

        if (btnSet.isClick())
        {
            saveCurrentSetting();
            editingValue = false;
        }

        if (btnEsc.isClick())
        {
            editingValue = false;
        }
        
    }

    // Изменение значения
    void adjustValue(float delta)
    {
        switch (currentMenu)
        {
        case 0: // Охлаждение
            switch (currentItem)
            {
            case 0:
                cooler.getSettings().targetTemp += delta;
                break;
            case 1:
                cooler.getSettings().hysteresis += delta;
                break;
            case 2:
                cooler.getSettings().minInterval += (int)delta;
                break;
            }
            break;

        case 1: // Миксер
            switch (currentItem)
            {
            case 0:
                mixer.getSettings().mode = (mixer.getSettings().mode + 1) % 3;
                break;
            case 1:
                mixer.getSettings().workTime += (int)delta;
                break;
            case 2:
                mixer.getSettings().idleTime += (int)delta;
                break;
            }
            break;

        case 2: // Мойка
            if (currentItem < 5)
            {
                washer.getSettings().stageTimes[currentItem] += (int)delta;
            }
            break;
        }
    }

    // Сохранение текущих настроек
    void saveCurrentSetting()
    {
        switch (currentMenu)
        {
        case 0:
            cooler.saveSettings();
            break;
        case 1:
            mixer.saveSettings();
            break;
        case 2:
            washer.saveSettings();
            break;
        }
        display.showMessage("Saved!");
        delay(1000);
        showCurrentMenu();
    }

public:
    // Конструктор
    ButtonMenuHandler(
        uint8_t upPin, uint8_t downPin, uint8_t setPin, uint8_t escPin,
        Display &displayRef, CoolerController &coolerRef,
        MixerController &mixerRef, WashingController &washerRef) : btnUp(upPin), btnDown(downPin), btnSet(setPin), btnEsc(escPin),
                                                                   display(displayRef), cooler(coolerRef), mixer(mixerRef), washer(washerRef)
    {
        // Настройка кнопокzz
        btnUp.setTickMode(AUTO);
        btnDown.setTickMode(AUTO);
        btnSet.setTickMode(AUTO);
        btnEsc.setTickMode(AUTO);
    }

    // Основной метод обновления
    void update()
    {

        // Активация меню по долгому нажатию SET
        if (btnSet.isHolded() && !menuActive && !editingValue)
        {
            menuActive = true;
            currentMenu = 0;
            showCurrentMenu();
            return;
        }

        if (!menuActive)
            return;

        if (editingValue)
        {
            editCurrentSetting();
            return;
        }

        // Навигация по меню
        if (btnUp.isClick())
        {
            currentMenu = (currentMenu == 0) ? 3 : currentMenu - 1;
            showCurrentMenu();
        }

        if (btnDown.isClick())
        {
            currentMenu = (currentMenu + 1) % 4;
            showCurrentMenu();
        }

        if (btnSet.isClick())
        {
            if (currentMenu == 3)
            {
                menuActive = false;
            }
            else
            {
                editCurrentSetting();
            }
        }

        if (btnEsc.isClick())
        {
            menuActive = false;
            editingValue = false;
        }
    }

    // Проверка активности меню
    bool isMenuActive() const
    {
        return menuActive || editingValue;
    }
};

// Инициализация PROGMEM строк
const char ButtonMenuHandler::menuItems[4][16] PROGMEM = {
    "Cooler Settings",
    "Mixer Settings",
    "Washer Settings",
    "Exit"};