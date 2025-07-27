/*#pragma once
#include "Display.h"
#include "ButtonHandler.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"
#include <GyverButton.h>

class MenuSystem {
private:
    Display* display;
    ButtonHandler* buttons;

    CoolerController* cooler;
    MixerController* mixer;
    WashingController* washer;
    
    bool menuActive = false;
    uint8_t currentMenu = 0;
    uint8_t currentItem = 0;
    bool editingValue = false;
    float editStep = 0.5f;
    
    const char* menuItems[4] = {
        "Cooler Settings",
        "Mixer Settings", 
        "Washer Settings",
        "Exit"
    };

    void showCurrentMenu() {
        display->showMessage(menuItems[currentMenu]);
    }

    void showCurrentValue() {
        char buf[17];
        switch(currentMenu) {
            case 0:
                switch(currentItem) {
                    case 0:
                        snprintf(buf, 16, "Target: %3.1f C", (double)cooler->getSettings().targetTemp);
                        break;
                    case 1:
                        snprintf(buf, 16, "Hysteresis: %3.1f", (double)cooler->getSettings().hysteresis);
                        break;
                    case 2:
                        snprintf(buf, 16, "Min interval: %d", cooler->getSettings().minInterval);
                        break;
                }
                break;
            case 1:
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
            case 2:
                if(currentItem < 5) {
                    snprintf(buf, 16, "%s: %d sec", 
                            washer->getStageTimeName(currentItem),
                            washer->getSettings().stageTimes[currentItem]);
                }
                break;
        }
        display->showMessage(buf);
    }

    void editCurrentSetting() {
        if(!editingValue) {
            editingValue = true;
            currentItem = 0;
            showCurrentValue();
            return;
        }

        if(buttons->i) {
            adjustValue(editStep);
        }
        if(buttons->isDownPressed()) {
            adjustValue(-editStep);
        }
        if(buttons->isSetPressed()) {
            saveCurrentSetting();
            editingValue = false;
        }
        if(buttons->isEscPressed()) {
            editingValue = false;
        }
    }

    void adjustValue(float delta) {
        switch(currentMenu) {
            case 0:
                switch(currentItem) {
                    case 0: cooler->getSettings().targetTemp += delta; break;
                    case 1: cooler->getSettings().hysteresis += delta; break;
                    case 2: cooler->getSettings().minInterval += (int)delta; break;
                }
                break;
            case 1:
                switch(currentItem) {
                    case 0: mixer->getSettings().mode = (mixer->getSettings().mode + 1) % 3; break;
                    case 1: mixer->getSettings().workTime += (int)delta; break;
                    case 2: mixer->getSettings().idleTime += (int)delta; break;
                }
                break;
            case 2:
                if(currentItem < 5) {
                    washer->getSettings().stageTimes[currentItem] += (int)delta;
                }
                break;
        }
        showCurrentValue();
    }

    void saveCurrentSetting() {
        switch(currentMenu) {
            case 0: cooler->saveSettings(); break;
            case 1: mixer->saveSettings(); break;
            case 2: washer->saveSettings(); break;
        }
        display->showMessage("Saved!");
        delay(1000);
    }

public:
    MenuSystem(Display* d, ButtonHandler* b, CoolerController* c, 
              MixerController* m, WashingController* w)
        : display(d), buttons(b), cooler(c), mixer(m), washer(w) {}

    void update() {
        if(buttons->isSetPressed() && !menuActive && !editingValue) {
            menuActive = true;
            currentMenu = 0;
            showCurrentMenu();
            buttons->resetLongPress();
            return;
        }

        if(!menuActive) return;

        if(editingValue) {
            editCurrentSetting();
            return;
        }

        if(buttons->isUpPressed()) {
            currentMenu = (currentMenu == 0) ? 3 : currentMenu - 1;
            showCurrentMenu();
        }

        if(buttons->isDownPressed()) {
            currentMenu = (currentMenu + 1) % 4;
            showCurrentMenu();
        }

        if(buttons->isSetPressed()) {
            if(currentMenu == 3) {
                menuActive = false;
            } else {
                editCurrentSetting();
            }
        }

        if(buttons->isEscPressed()) {
            menuActive = false;
            editingValue = false;
        }
    }

    bool isActive() const { return menuActive || editingValue; }
}; */