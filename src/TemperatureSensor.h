#pragma once
#include <GyverNTC.h> // Библиотека для работы с NTC термистором

/*
  Класс для работы с датчиком температуры на NTC термисторе
  Реализует фильтрацию показаний и калибровку
*/
class TemperatureSensor {
private:
    GyverNTC ntc;              // Объект для работы с термистором
    float filteredTemp = 0.0;  // Отфильтрованное значение температуры
    float alpha = 0.1;         // Коэффициент фильтрации (0.1 = 10%)
    float calibrationOffset = 0.0; // Поправка калибровки

public:
    /*
      Конструктор:
      pin - аналоговый пин подключения
      R - сопротивление резистора в делителе (Ом)
      B - коэффициент B термистора
      Rt - сопротивление термистора при 25°C
    */
    TemperatureSensor(uint8_t pin, int R, int B) 
        : ntc(pin, R, B) {} // Инициализация объекта NTC

    // Обновление показаний (вызывать в loop)
    void update() {
        float rawTemp = ntc.getTempAverage(); // Получаем среднее значение
        // Применяем фильтр низких частот
        filteredTemp = alpha * rawTemp + (1 - alpha) * filteredTemp;
    }

    // Получение текущей температуры с учетом калибровки
    float getTemp() const { 
        return filteredTemp + calibrationOffset; 
    }

    // Проверка исправности датчика
    bool isSensorOK() const {
        // Температура должна быть в разумных пределах
        return filteredTemp > -50.0 && filteredTemp < 150.0;
    }

    // Установка коэффициента фильтрации (0.01-0.3)
    void setFilterCoefficient(float a) {
        alpha = constrain(a, 0.01f, 0.3f); // Ограничиваем значение
    }

    // Калибровка датчика по известной температуре
    void calibrate(float referenceTemp) {
        calibrationOffset = referenceTemp - filteredTemp;
    }
};