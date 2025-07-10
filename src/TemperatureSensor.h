#pragma once
#include <GyverNTC.h> // Подключение библиотеки для NTC термистора

class TemperatureSensor
{
private:
    GyverNTC ntc;                  // Объект термистора
    float filteredTemp;      // Отфильтрованное значение температуры
    float alpha = 0.1;             // Коэффициент фильтра (0.1 - 10% нового значения)
    float calibrationOffset = 0.0; // Смещение для калибровки

public:
    // Конструктор с параметрами термистора
    TemperatureSensor(uint8_t pin, int R, int B, int Rt)
        : ntc(pin, R, B, Rt) {} // Инициализация объекта термистора

    // Обновление значения температуры
    void update()
    {
        float rawTemp = ntc.getTempAverage();  // Получение сырого значения
        //  Экспоненциальное сглаживание
        filteredTemp = alpha * rawTemp + (1 - alpha) * filteredTemp;
    }

    // Получение текущей температуры (с учетом калибровки)
    float getTemp() const
    {
        return filteredTemp + calibrationOffset;
    }

    // Проверка исправности датчика
    bool isSensorOK() const
    {
        return filteredTemp > -50 && filteredTemp < 150; // Разумный диапазон
    }

    // Установка коэффициента фильтра
    void setFilterCoefficient(float a)
    {
        alpha = constrain(a, 0.01, 0.3); // Ограничение диапазона
    }

    // Калибровка датчика по эталонному значению
    void calibrate(float referenceTemp)
    {
        calibrationOffset = referenceTemp - filteredTemp;
    }
};