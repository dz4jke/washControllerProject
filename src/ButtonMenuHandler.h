#pragma once
#include <GyverButton.h>
#include "Display.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"
#include "TemperatureSensor.h"

// Перечисления событий для обработки кнопок
enum MenuEvent {
    EVENT_NONE,
    EVENT_UP,
    EVENT_DOWN,
    EVENT_SELECT,
    EVENT_BACK,
    EVENT_HOLD_UP,
    EVENT_HOLD_DOWN // Добавлено для ускоренной прокрутки
};

// Перечисления состояний меню
enum MenuState {
    STATE_MAIN_SCREEN,
    STATE_MAIN_MENU,
    STATE_COOLER_MENU,
    STATE_MIXER_MENU,
    STATE_WASHER_MENU,
    STATE_TEST_MENU,
    STATE_EDIT_VALUE
};

// Структура пункта меню
struct MenuItem {
    const char* text;
    MenuState nextState;
    float* value; // Указатель на значение, которое редактируется
    float min;
    float max;
    float step;
    const char* unit;
};

class ButtonMenuHandler {
private:
    GButton btnUp, btnDown, btnSet, btnEsc;
    Display& display;
    CoolerController& cooler;
    MixerController& mixer;
    WashingController& washer;
    TemperatureSensor& tempSensor; // Добавлен объект для датчика температуры

    MenuState currentState = STATE_MAIN_SCREEN;
    MenuState previousState = STATE_MAIN_SCREEN; // Для возврата на предыдущий уровень
    uint8_t currentItem = 0;
    float editValue = 0.0f; // Текущее редактируемое значение

    const MenuItem* currentMenu = nullptr; // Указатель на текущий активный массив меню
    uint8_t menuSize = 0; // Размер текущего меню

    // Пункты меню теперь являются членами класса, а не статическими,
    // что позволяет им ссылаться на экземпляры контроллеров.
    const MenuItem mainMenu[4];
    const MenuItem coolerMenu[3];
    const MenuItem mixerMenu[3];
    const MenuItem washerMenu[5];
    const MenuItem testMenu[8];

    // Таймер для автоматического возврата на главный экран
    unsigned long returnTimer = 0;
    const unsigned long RETURN_TIMEOUT = 30000; // 30 секунд бездействия

    /*
     * Опрашивает кнопки и возвращает соответствующее событие
     */
    MenuEvent getEvent() {
        if (btnUp.isClick()) return EVENT_UP;
        if (btnDown.isClick()) return EVENT_DOWN;
        if (btnSet.isClick()) return EVENT_SELECT;
        if (btnEsc.isClick()) return EVENT_BACK;
        if (btnUp.isHold()) return EVENT_HOLD_UP; // Удержание кнопки для ускоренной прокрутки
        if (btnDown.isHold()) return EVENT_HOLD_DOWN; // Удержание кнопки для ускоренной прокрутки
        return EVENT_NONE;
    }

    /*
     * Переводит систему в новое состояние меню
     */
    void goToState(MenuState newState) {
        previousState = currentState; // Сохраняем текущее состояние как предыдущее
        currentState = newState;
        currentItem = 0; // При входе в новое меню всегда начинаем с первого элемента

        switch (currentState) {
            case STATE_MAIN_MENU:
                currentMenu = mainMenu;
                menuSize = 4;
                showMenu();
                break;
            case STATE_COOLER_MENU:
                currentMenu = coolerMenu;
                menuSize = 3;
                showMenu();
                break;
            case STATE_MIXER_MENU:
                currentMenu = mixerMenu;
                menuSize = 3;
                showMenu();
                break;
            case STATE_WASHER_MENU:
                currentMenu = washerMenu;
                menuSize = 5;
                showMenu();
                break;
            case STATE_TEST_MENU:
                currentMenu = testMenu;
                menuSize = 8;
                showMenu();
                break;
            case STATE_EDIT_VALUE:
                // При входе в режим редактирования, загружаем текущее значение
                if (currentMenu[currentItem].value != nullptr) {
                    editValue = *currentMenu[currentItem].value;
                } else {
                    editValue = 0.0f; // Дефолтное значение, если указатель не задан
                }
                showEditValue();
                break;
            case STATE_MAIN_SCREEN:
                showMainScreen(); // Обновляем главный экран
                break;
            default:
                // Если состояние не определено, возвращаемся на главный экран
                goToState(STATE_MAIN_SCREEN);
                break;
        }
    }

    /*
     * Отображает текущее меню на дисплее
     */
    void showMenu() {
        char buf[17];
        snprintf(buf, sizeof(buf), "%s", currentMenu[currentItem].text);
        display.showMenuScreen(buf, currentItem + 1, menuSize);
    }

    /*
     * Отображает экран редактирования значения
     */
    void showEditValue() {
        char buf[17];
        char valStr[10];
        dtostrf(editValue, 4, 1, valStr); // Форматируем float в строку
        snprintf(buf, sizeof(buf), "%s: %s %s",
                 currentMenu[currentItem].text,
                 valStr,
                 currentMenu[currentItem].unit);
        display.showMessage(buf);
    }

    /*
     * Выполняет действие для тестового меню
     */
    void handleTestAction(uint8_t item) {
        // Здесь можно добавить логику включения/выключения для тестовых элементов
        // Пример:
        bool state = false; // Состояние по умолчанию - выключено
        if (currentMenu[item].value != nullptr) {
            state = !(*currentMenu[item].value); // Переключаем состояние
            *currentMenu[item].value = state;
        }

        switch (item) {
            case 0: cooler.setCompressorState(state); break;
            case 1: mixer.setMixerState(state); break;
            case 2: washer.setWashPump(state); break;
            case 3: washer.setDrainValve(state); break;
            case 4: washer.setColdWaterValve(state); break;
            case 5: washer.setHotWaterValve(state); break;
            case 6: washer.setAlkaliPump(state); break;
            case 7: washer.setAcidPump(state); break;
        }
        
        display.showMessage(state ? "ON" : "OFF"); // Показываем состояние
        delay(1000);
        showMenu(); // Возвращаемся в тестовое меню
    }

    /*
     * Сохраняет отредактированное значение в соответствующий контроллер
     */
    void saveCurrentValue() {
        if (currentMenu[currentItem].value != nullptr) {
            *currentMenu[currentItem].value = editValue; // Сохраняем значение
        }
        
        // Сохраняем настройки в соответствующий контроллер
        switch (previousState) {
            case STATE_COOLER_MENU: cooler.saveSettings(); break;
            case STATE_MIXER_MENU: mixer.saveSettings(); break;
            case STATE_WASHER_MENU: washer.saveSettings(); break;
            default: break;
        }
    }

public:
    /*
     * Конструктор класса
     */
    ButtonMenuHandler(uint8_t upPin, uint8_t downPin, uint8_t setPin, uint8_t escPin,
                      Display& displayRef, CoolerController& coolerRef,
                      MixerController& mixerRef, WashingController& washerRef,
                      TemperatureSensor& tempSensorRef)
        : btnUp(upPin), btnDown(downPin), btnSet(setPin), btnEsc(escPin),
          display(displayRef), cooler(coolerRef), mixer(mixerRef), washer(washerRef),
          tempSensor(tempSensorRef),
          // Инициализация массивов меню с использованием ссылок на параметры контроллеров
          mainMenu{
              {"Cooler Settings", STATE_COOLER_MENU, nullptr, 0, 0, 0, ""},
              {"Mixer Settings", STATE_MIXER_MENU, nullptr, 0, 0, 0, ""},
              {"Washer Settings", STATE_WASHER_MENU, nullptr, 0, 0, 0, ""},
              {"Test Mechanisms", STATE_TEST_MENU, nullptr, 0, 0, 0, ""}
          },
          coolerMenu{
              {"Target Temp", STATE_EDIT_VALUE, &cooler.getSettings().targetTemp, -10, 30, 0.5, "C"},
              {"Hysteresis", STATE_EDIT_VALUE, &cooler.getSettings().hysteresis, 0.5, 5, 0.1, "C"},
              {"Min Interval", STATE_EDIT_VALUE, (float*)&cooler.getSettings().minInterval, 10, 600, 10, "s"} // Приведение типа
          },
          mixerMenu{
              {"Mode", STATE_EDIT_VALUE, (float*)&mixer.getSettings().mode, 0, 2, 1, ""}, // Приведение типа
              {"Work Time", STATE_EDIT_VALUE, (float*)&mixer.getSettings().workTime, 10, 600, 10, "s"}, // Приведение типа
              {"Idle Time", STATE_EDIT_VALUE, (float*)&mixer.getSettings().idleTime, 10, 600, 10, "s"} // Приведение типа
          },
          washerMenu{
              {"Stage 1 Time", STATE_EDIT_VALUE, (float*)&washer.getSettings().stageTimes[0], 5, 300, 5, "s"},
              {"Stage 2 Time", STATE_EDIT_VALUE, (float*)&washer.getSettings().stageTimes[1], 5, 300, 5, "s"},
              {"Stage 3 Time", STATE_EDIT_VALUE, (float*)&washer.getSettings().stageTimes[2], 5, 300, 5, "s"},
              {"Stage 4 Time", STATE_EDIT_VALUE, (float*)&washer.getSettings().stageTimes[3], 5, 300, 5, "s"},
              {"Stage 5 Time", STATE_EDIT_VALUE, (float*)&washer.getSettings().stageTimes[4], 5, 300, 5, "s"}
          },
          testMenu{
              // Для тестового меню, value теперь указывает на временные состояния для переключения
              {"Compressor", STATE_TEST_MENU, new float(false), 0, 1, 1, ""}, // Временные состояния для теста
              {"Mixer", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Wash Pump", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Drain Valve", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Cold Water", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Hot Water", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Alkali Pump", STATE_TEST_MENU, new float(false), 0, 1, 1, ""},
              {"Acid Pump", STATE_TEST_MENU, new float(false), 0, 1, 1, ""}
          }
    {
        btnUp.setTickMode(AUTO);
        btnDown.setTickMode(AUTO);
        btnSet.setTickMode(AUTO);
        btnEsc.setTickMode(AUTO);
        
    }

    // Деструктор для освобождения памяти, выделенной для testMenu
    ~ButtonMenuHandler() {
        for (uint8_t i = 0; i < 8; ++i) {
            delete testMenu[i].value;
        }
    }

    /*
     * Основной метод обновления состояния меню
     */
    void update() {
        MenuEvent event = getEvent();
        
        // Сбрасываем таймер активности при любом событии кнопки
        if (event != EVENT_NONE) {
            returnTimer = millis();
        }

        // Автоматический возврат на главный экран при бездействии
        if (currentState != STATE_MAIN_SCREEN && (millis() - returnTimer > RETURN_TIMEOUT)) {
            goToState(STATE_MAIN_SCREEN);
            return;
        }

        switch (currentState) {
            case STATE_MAIN_SCREEN:
                // Переход в главное меню по нажатию "SET"
                if (btnSet.isClick()) {
                    goToState(STATE_MAIN_MENU);
                }
                break;

            case STATE_MAIN_MENU:
            case STATE_COOLER_MENU:
            case STATE_MIXER_MENU:
            case STATE_WASHER_MENU:
            case STATE_TEST_MENU:
                if (event == EVENT_UP && currentItem > 0) {
                    currentItem--;
                    showMenu();
                } else if (event == EVENT_DOWN && currentItem < menuSize - 1) {
                    currentItem++;
                    showMenu();
                } else if (event == EVENT_SELECT) {
                    if (currentState == STATE_TEST_MENU) {
                        handleTestAction(currentItem);
                    } else {
                        goToState(currentMenu[currentItem].nextState);
                    }
                } else if (event == EVENT_BACK) {
                    // Возврат на предыдущий уровень (MAIN_SCREEN для MAIN_MENU)
                    if (currentState == STATE_MAIN_MENU) {
                         goToState(STATE_MAIN_SCREEN);
                    } else {
                        // Для подменю (Cooler, Mixer, Washer, Test) возврат в MAIN_MENU
                        goToState(STATE_MAIN_MENU);
                    }
                }
                break;

            case STATE_EDIT_VALUE:
                if (event == EVENT_UP || event == EVENT_HOLD_UP) {
                    editValue = constrain(editValue + currentMenu[currentItem].step,
                                         currentMenu[currentItem].min,
                                         currentMenu[currentItem].max);
                    showEditValue();
                } else if (event == EVENT_DOWN || event == EVENT_HOLD_DOWN) {
                    editValue = constrain(editValue - currentMenu[currentItem].step,
                                         currentMenu[currentItem].min,
                                         currentMenu[currentItem].max);
                    showEditValue();
                } else if (event == EVENT_SELECT) {
                    saveCurrentValue();
                    display.showMessage("Saved!");
                    delay(1000);
                    goToState(previousState); // Возвращаемся на предыдущий уровень меню
                } else if (event == EVENT_BACK) {
                    goToState(previousState); // Отмена редактирования и возврат
                }
                break;
        }
    }

    /*
     * Проверяет, активно ли меню (не главный экран)
     */
    bool isMenuActive() const {
        return currentState != STATE_MAIN_SCREEN;
    }
    
    /*
     * Обновляет главный экран (вызывается из loop, когда меню не активно)
     */
    void showMainScreen() {
      display.showMainScreen(tempSensor.getTemp(), mixer.isActive(), cooler.isRunning());
    }
};
