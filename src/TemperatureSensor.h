#pragma once
#include <GyverNTC.h>

// Класс датчика температуры
class TemperatureSensor {
private:
    GyverNTC ntc;
    float filteredTemp;
    const float alpha;
    float calibrationOffset;
    float one_minus_alpha;

public:
    // Конструктор
    TemperatureSensor(uint8_t pin, int R = 10000, int B = 3950, float a = 0.1f) 
        : ntc(pin, R, B), 
          filteredTemp(0.0f), 
          alpha(constrain(a, 0.01f, 0.3f)), 
          calibrationOffset(0.0f),
          one_minus_alpha(1.0f - alpha) {}

    // Обновление показаний
    void update() {
        float rawTemp = ntc.getTemp();  
        filteredTemp += alpha * (rawTemp - filteredTemp);
    }

    // Получение температуры
    float getTemp() const { 
        return filteredTemp + calibrationOffset; 
    }

    // Проверка исправности датчика
    bool isSensorOK() const {
        return !(filteredTemp <= -50.0f || filteredTemp >= 150.0f);
    }

    // Калибровка
    void calibrate(float referenceTemp) {
        calibrationOffset = referenceTemp - filteredTemp;
    }
};