#pragma once
#include <GyverNTC.h>

/*
 * Класс для работы с датчиком температуры
 * Реализует:
 * - Фильтрацию показаний
 * - Проверку исправности датчика
 * - Калибровку
 */
class TemperatureSensor {
private:
    GyverNTC ntc;               // Объект датчика
    float filteredTemp;          // Отфильтрованное значение
    const float alpha;          // Коэффициент фильтра (0.0-1.0)
    float calibrationOffset;    // Калибровочное смещение
    const float one_minus_alpha; // 1 - alpha (оптимизация вычислений)

public:
    /*
     * Конструктор
     * pin - аналоговый пин датчика
     * R - сопротивление резистора (Ом)
     * B - коэффициент B термистора
     * a - коэффициент фильтра (0.0-1.0)
     */
    TemperatureSensor(uint8_t pin, int R = 10000, int B = 3950, float a = 0.1f) 
        : ntc(pin, R, B), 
          filteredTemp(0.0f), 
          alpha(constrain(a, 0.01f, 0.3f)), 
          calibrationOffset(0.0f),
          one_minus_alpha(1.0f - alpha) {}

    /*
     * Обновление показаний (должно вызываться регулярно)
     */
    void update() {
        float rawTemp = ntc.getTemp();  
        // Экспоненциальное скользящее среднее
        filteredTemp = alpha * rawTemp + one_minus_alpha * filteredTemp;
    }

    /*
     * Получение текущей температуры
     * Возвращает температуру в °C с учетом калибровки
     */
    float getTemp() const { 
        return filteredTemp + calibrationOffset; 
    }

    /*
     * Проверка исправности датчика
     * Возвращает false при выходе за допустимый диапазон
     */
    bool isSensorOK() const {
        return !(filteredTemp <= -50.0f || filteredTemp >= 150.0f);
    }

    /*
     * Калибровка датчика
     * referenceTemp - эталонная температура
     */
    void calibrate(float referenceTemp) {
        calibrationOffset = referenceTemp - filteredTemp;
    }
};