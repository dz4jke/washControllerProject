#pragma once
#include <GyverButton.h>
#include "Display.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"

enum MenuEvent {
    EVENT_NONE,
    EVENT_UP,
    EVENT_DOWN,
    EVENT_SELECT,
    EVENT_BACK
};

enum MenuState {
    STATE_MAIN_MENU,
    STATE_COOLER_MENU,
    STATE_MIXER_MENU,
    STATE_WASHER_MENU,
    STATE_TEST_MENU,
    STATE_EDIT_VALUE
};

struct MenuItem {
    const char* text;
    MenuState nextState;
    float* value;
    float min;
    float max;
    float step;
    const char* unit;
};

class ButtonMenuHandler {
private:
    GButton btnUp, btnDown, btnSet, btnEsc;
    Display &display;
    CoolerController &cooler;
    MixerController &mixer;
    WashingController &washer;

    MenuState currentState = STATE_MAIN_MENU;
    uint8_t currentItem = 0;
    float editValue;
    const MenuItem* currentMenu = nullptr;
    uint8_t menuSize = 0;

    // Главное меню
    static const MenuItem mainMenu[];
    static const uint8_t MAIN_MENU_SIZE;

    // Меню компрессора (без привязки к экземпляру)
    static const MenuItem coolerMenu[];
    static const uint8_t COOLER_MENU_SIZE;

    // Меню миксера (без привязки к экземпляру)
    static const MenuItem mixerMenu[];
    static const uint8_t MIXER_MENU_SIZE;

    // Меню мойки (без привязки к экземпляру)
    static const MenuItem washerMenu[];
    static const uint8_t WASHER_MENU_SIZE;

    // Меню тестирования
    static const MenuItem testMenu[];
    static const uint8_t TEST_MENU_SIZE;

    MenuEvent getEvent() {
        if (btnUp.isClick()) return EVENT_UP;
        if (btnDown.isClick()) return EVENT_DOWN;
        if (btnSet.isClick()) return EVENT_SELECT;
        if (btnEsc.isClick()) return EVENT_BACK;
        return EVENT_NONE;
    }

    void showMenu() {
        char buf[21];
        snprintf(buf, sizeof(buf), "%s", currentMenu[currentItem].text);
        display.showMenuScreen(buf, currentItem + 1, menuSize);
    }

    void showEditValue() {
        char buf[21];
        char valStr[10];
        dtostrf(editValue, 4, 1, valStr);
        snprintf(buf, sizeof(buf), "%s: %s %s", 
                currentMenu[currentItem].text, 
                valStr, 
                currentMenu[currentItem].unit);
        display.showMessage(buf);
    }

    void enterState(MenuState newState) {
        currentState = newState;
        currentItem = 0;
        
        switch(currentState) {
            case STATE_MAIN_MENU:
                currentMenu = mainMenu;
                menuSize = MAIN_MENU_SIZE;
                break;
            case STATE_COOLER_MENU:
                currentMenu = coolerMenu;
                menuSize = COOLER_MENU_SIZE;
                break;
            case STATE_MIXER_MENU:
                currentMenu = mixerMenu;
                menuSize = MIXER_MENU_SIZE;
                break;
            case STATE_WASHER_MENU:
                currentMenu = washerMenu;
                menuSize = WASHER_MENU_SIZE;
                break;
            case STATE_TEST_MENU:
                currentMenu = testMenu;
                menuSize = TEST_MENU_SIZE;
                break;
            case STATE_EDIT_VALUE:
                // Здесь мы получаем актуальные значения из соответствующих контроллеров
                switch(currentState) {
                    case STATE_COOLER_MENU:
                        switch(currentItem) {
                            case 0: editValue = cooler.getSettings().targetTemp; break;
                            case 1: editValue = cooler.getSettings().hysteresis; break;
                            case 2: editValue = cooler.getSettings().minInterval; break;
                        }
                        break;
                    case STATE_MIXER_MENU:
                        switch(currentItem) {
                            case 1: editValue = mixer.getSettings().workTime; break;
                            case 2: editValue = mixer.getSettings().idleTime; break;
                        }
                        break;
                    case STATE_WASHER_MENU:
                        editValue = washer.getSettings().stageTimes[currentItem];
                        break;
                    default:
                        break;
                }
                showEditValue();
                return;
        }
        showMenu();
    }

    void handleTestAction(uint8_t item) {
        switch(item) {
            case 0: cooler.setCompressorState(true); break;
            case 1: mixer.setMixerState(true); break;
            case 2: washer.setWashPump(true); break;
            case 3: washer.setDrainValve(true); break;
            case 4: washer.setColdWaterValve(true); break;
            case 5: washer.setHotWaterValve(true); break;
            case 6: washer.setAlkaliPump(true); break;
            case 7: washer.setAcidPump(true); break;
        }
        display.showMessage("Activated");
        delay(1000);
        showMenu();
    }

    void saveCurrentValue() {
        switch(currentState) {
            case STATE_COOLER_MENU:
                switch(currentItem) {
                    case 0: cooler.getSettings().targetTemp = editValue; break;
                    case 1: cooler.getSettings().hysteresis = editValue; break;
                    case 2: cooler.getSettings().minInterval = editValue; break;
                }
                cooler.saveSettings();
                break;
            case STATE_MIXER_MENU:
                switch(currentItem) {
                    case 1: mixer.getSettings().workTime = editValue; break;
                    case 2: mixer.getSettings().idleTime = editValue; break;
                }
                mixer.saveSettings();
                break;
            case STATE_WASHER_MENU:
                washer.getSettings().stageTimes[currentItem] = editValue;
                washer.saveSettings();
                break;
            default:
                break;
        }
    }

public:
    ButtonMenuHandler(uint8_t upPin, uint8_t downPin, uint8_t setPin, uint8_t escPin,
                    Display &displayRef, CoolerController &coolerRef,
                    MixerController &mixerRef, WashingController &washerRef)
        : btnUp(upPin), btnDown(downPin), btnSet(setPin), btnEsc(escPin),
          display(displayRef), cooler(coolerRef), mixer(mixerRef), washer(washerRef) 
    {
        btnUp.setTickMode(AUTO);
        btnDown.setTickMode(AUTO);
        btnSet.setTickMode(AUTO);
        btnEsc.setTickMode(AUTO);
        
        enterState(STATE_MAIN_MENU);
    }

    void update() {
        MenuEvent event = getEvent();
        if (event == EVENT_NONE) return;

        switch(currentState) {
            case STATE_MAIN_MENU:
            case STATE_COOLER_MENU:
            case STATE_MIXER_MENU:
            case STATE_WASHER_MENU:
            case STATE_TEST_MENU:
                if (event == EVENT_UP && currentItem > 0) {
                    currentItem--;
                    showMenu();
                }
                else if (event == EVENT_DOWN && currentItem < menuSize - 1) {
                    currentItem++;
                    showMenu();
                }
                else if (event == EVENT_SELECT) {
                    if (currentState == STATE_TEST_MENU) {
                        handleTestAction(currentItem);
                    } else {
                        enterState(currentMenu[currentItem].nextState);
                    }
                }
                else if (event == EVENT_BACK) {
                    enterState(STATE_MAIN_MENU);
                }
                break;
                
            case STATE_EDIT_VALUE:
                if (event == EVENT_UP) {
                    editValue = constrain(editValue + currentMenu[currentItem].step,
                                        currentMenu[currentItem].min,
                                        currentMenu[currentItem].max);
                    showEditValue();
                }
                else if (event == EVENT_DOWN) {
                    editValue = constrain(editValue - currentMenu[currentItem].step,
                                        currentMenu[currentItem].min,
                                        currentMenu[currentItem].max);
                    showEditValue();
                }
                else if (event == EVENT_SELECT) {
                    saveCurrentValue();
                    display.showMessage("Saved!");
                    delay(1000);
                    enterState(currentState);
                }
                else if (event == EVENT_BACK) {
                    enterState(currentState);
                }
                break;
        }
    }

    bool isMenuActive() const {
        return currentState != STATE_MAIN_MENU;
    }
};

// Инициализация меню (без привязки к конкретным экземплярам)
const MenuItem ButtonMenuHandler::mainMenu[] PROGMEM = {
    {"Cooler Settings", STATE_COOLER_MENU, nullptr, 0, 0, 0, ""},
    {"Mixer Settings", STATE_MIXER_MENU, nullptr, 0, 0, 0, ""},
    {"Washer Settings", STATE_WASHER_MENU, nullptr, 0, 0, 0, ""},
    {"Test Mechanisms", STATE_TEST_MENU, nullptr, 0, 0, 0, ""}
};
const uint8_t ButtonMenuHandler::MAIN_MENU_SIZE = 4;

const MenuItem ButtonMenuHandler::coolerMenu[] PROGMEM = {
    {"Target Temp", STATE_EDIT_VALUE, nullptr, -10, 30, 0.5, "C"},
    {"Hysteresis", STATE_EDIT_VALUE, nullptr, 0.5, 5, 0.1, "C"},
    {"Min Interval", STATE_EDIT_VALUE, nullptr, 10, 600, 10, "s"}
};
const uint8_t ButtonMenuHandler::COOLER_MENU_SIZE = 3;

const MenuItem ButtonMenuHandler::mixerMenu[] PROGMEM = {
    {"Mode", STATE_MIXER_MENU, nullptr, 0, 2, 1, ""},
    {"Work Time", STATE_EDIT_VALUE, nullptr, 10, 600, 10, "s"},
    {"Idle Time", STATE_EDIT_VALUE, nullptr, 10, 600, 10, "s"}
};
const uint8_t ButtonMenuHandler::MIXER_MENU_SIZE = 3;

const MenuItem ButtonMenuHandler::washerMenu[] PROGMEM = {
    {"Stage 1 Time", STATE_EDIT_VALUE, nullptr, 5, 300, 5, "s"},
    {"Stage 2 Time", STATE_EDIT_VALUE, nullptr, 5, 300, 5, "s"},
    {"Stage 3 Time", STATE_EDIT_VALUE, nullptr, 5, 300, 5, "s"},
    {"Stage 4 Time", STATE_EDIT_VALUE, nullptr, 5, 300, 5, "s"},
    {"Stage 5 Time", STATE_EDIT_VALUE, nullptr, 5, 300, 5, "s"}
};
const uint8_t ButtonMenuHandler::WASHER_MENU_SIZE = 5;

const MenuItem ButtonMenuHandler::testMenu[] PROGMEM = {
    {"Compressor", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Mixer", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Wash Pump", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Drain Valve", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Cold Water", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Hot Water", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Alkali Pump", STATE_TEST_MENU, nullptr, 0, 0, 0, ""},
    {"Acid Pump", STATE_TEST_MENU, nullptr, 0, 0, 0, ""}
};
const uint8_t ButtonMenuHandler::TEST_MENU_SIZE = 8;