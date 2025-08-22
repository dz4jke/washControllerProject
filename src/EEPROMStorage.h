#pragma once
#include <EEPROM.h>
#include <Arduino.h> // Для size_t

/*
 * Класс для работы с EEPROM
 * Реализует:
 * - Чтение/запись любых типов данных
 * - Очистку EEPROM
 * - Оптимизированную запись (только при изменении)
 */
class EEPROMStorage {
public:
    /*
     * Чтение данных из EEPROM
     * address - начальный адрес в EEPROM
     * data - ссылка на объект, в который будут считаны данные
     */
    template <typename T>
    static void read(int address, T &data) {
        EEPROM.get(address, data);
    }

    /*
     * Запись данных в EEPROM (только если изменились)
     * address - начальный адрес в EEPROM
     * data - ссылка на объект, данные которого будут записаны
     */
    template <typename T>
    static void write(int address, const T &data) {
        // Читаем текущие данные из EEPROM
        T currentData;
        EEPROM.get(address, currentData);

        // Сравниваем новые данные с текущими
        if (memcmp(&currentData, &data, sizeof(T)) != 0) {
            // Если данные отличаются, записываем новые
            EEPROM.put(address, data);
        }
    }

    /*
     * Очистка всей EEPROM памяти (запись 0 во все ячейки)
     * Внимание: это может быть медленной операцией и сократить срок службы EEPROM!
     * Рекомендуется использовать только для сброса до заводских настроек.
     */
    static void clear() {
        for (size_t i = 0; i < EEPROM.length(); ++i) {
            if (EEPROM.read(i) != 0) { // Записываем 0 только если значение не 0
                EEPROM.write(i, 0);
            }
        }
    }

private:
    // Запрещаем создание экземпляров класса, так как это статический класс
    EEPROMStorage() = delete; 
};
