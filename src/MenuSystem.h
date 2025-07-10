#pragma once
#include "Display.h"
#include "ButtonHandler.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"

class MenuSystem {
private:
    Display* display;  // Указатель на дисплей
    ButtonHandler* buttons;  // Указатель на обработчик кнопок
    CoolerController* cooler;  // Указатель на контроллер холодильника
    MixerController* mixer;  // Указатель на контроллер мешалки
    WashingController* washer;  // Указатель на контроллер мойки
    
    bool menuActive = false;  // Флаг активности меню
    uint8_t currentMenu = 0;  // Текущий выбранный пункт меню
    uint8_t currentItem = 0;  // Текущий выбранный параметр
    bool editingValue = false;  // Флаг редактирования значения
    float editStep = 0.5f;  // Шаг изменения значения
    
    // Пункты главного меню
    const char* menuItems[4] = {
        "Cooler Settings",
        "Mixer Settings", 
        "Washer Settings",
        "Exit"
    };

    // Показать текущий пункт меню
    void showCurrentMenu() {
        display->showMessage(menuItems[currentMenu]);
    }

    // Показать текущее значение параметра
    void showCurrentValue() {
        char buf[17];
        switch(currentMenu) {
            case 0: // Настройки холодильника
                switch(currentItem) {
                    case 0:
                        snprintf(buf, 16, "Target: %3.1f C", cooler->getSettings().targetTemp);
                        break;
                    case 1:
                        snprintf(buf, 16, "Hysteresis: %3.1f", cooler->getSettings().hysteresis);
                        break;
                    case 2:
                        snprintf(buf, 16, "Min interval: %d", cooler->getSettings().minInterval);
                        break;
                }
                break;
            case 1: // Настройки мешалки
                switch(currentItem) {
                    case 0:
                        snprintf(buf, 16, "Mode: %d", mixer->getSettings().mode);
                        break;
                    case 1:
                        snprintf(buf, 16, "Work time: %d", mixer->getSettings().workTime);
                        break;
                    case 2:
                        snprintf(buf, 16, "Idle time: %d", mixer->getSettings().idleTime);
                        break;
                }
                break;
            case 2: // Настройки мойки
                if(currentItem < 5) {
                    snprintf(buf, 16, "%s: %d sec", 
                            washer->getStageTimeName(currentItem),
                            washer->getSettings().stageTimes[currentItem]);
                }
                break;
        }
        display->showMessage(buf);
    }

    // Редактирование текущего значения
    void editCurrentSetting() {
        if(!editingValue) {  // Если только начали редактирование
            editingValue = true;
            currentItem = 0;
            showCurrentValue();
            return;
        }

        // Обработка нажатий кнопок при редактировании
        if(buttons->isUpPressed()) {
            adjustValue(editStep);  // Увеличение значения
        }
        if(buttons->isDownPressed()) {
            adjustValue(-editStep);  // Уменьшение значения
        }
        if(buttons->isSetPressed()) {
            saveCurrentSetting();  // Сохранение
            editingValue = false;
        }
        if(buttons->isEscPressed()) {
            editingValue = false;  // Отмена без сохранения
        }
    }

    // Изменение значения параметра
    void adjustValue(float delta) {
        switch(currentMenu) {
            case 0: // Холодильник
                switch(currentItem) {
                    case 0: cooler->getSettings().targetTemp += delta; break;
                    case 1: cooler->getSettings().hysteresis += delta; break;
                    case 2: cooler->getSettings().minInterval += (int)delta; break;
                }
                break;
            case 1: // Мешалка
                switch(currentItem) {
                    case 0: mixer->getSettings().mode = (mixer->getSettings().mode + 1) % 3; break;
                    case 1: mixer->getSettings().workTime += (int)delta; break;
                    case 2: mixer->getSettings().idleTime += (int)delta; break;
                }
                break;
            case 2: // Мойка
                if(currentItem < 5) {
                    washer->getSettings().stageTimes[currentItem] += (int)delta;
                }
                break;
        }
        showCurrentValue();  // Обновление отображения
    }

    // Сохранение текущих настроек
    void saveCurrentSetting() {
        switch(currentMenu) {
            case 0: cooler->saveSettings(); break;
            case 1: mixer->saveSettings(); break;
            case 2: washer->saveSettings(); break;
        }
        display->showMessage("Saved!");  // Подтверждение сохранения
        delay(1000);
    }

public:
    // Конструктор
    MenuSystem(Display* d, ButtonHandler* b, CoolerController* c, 
              MixerController* m, WashingController* w)
        : display(d), buttons(b), cooler(c), mixer(m), washer(w) {}

    // Основной метод обновления меню
    void update() {
        // Активация меню по длительному нажатию
        if(buttons->isSetPressed() && !menuActive && !editingValue) {
            menuActive = true;
            currentMenu = 0;
            showCurrentMenu();
            buttons->resetLongPress();
            return;
        }

        if(!menuActive) return;  // Если меню не активно - выход

        if(editingValue) {  // Если в режиме редактирования
            editCurrentSetting();
            return;
        }

        // Навигация по меню
        if(buttons->isUpPressed()) {
            if(currentMenu == 0) currentMenu = 3;
            else currentMenu--;
            showCurrentMenu();
        }

        if(buttons->isDownPressed()) {
            currentMenu = (currentMenu + 1) % 4;
            showCurrentMenu();
        }

        // Выбор пункта меню
        if(buttons->isSetPressed()) {
            if(currentMenu == 3) {  // Выход
                menuActive = false;
            } else {
                editCurrentSetting();  // Редактирование
            }
        }

        // Отмена
        if(buttons->isEscPressed()) {
            menuActive = false;
            editingValue = false;
        }
    }

    // Проверка активности меню
    bool isActive() const { return menuActive || editingValue; }
};