#pragma once


#include <Adafruit_TSL2591/Adafruit_TSL2591.h>
#include <cstdint>

class SensorTSL2591
{
    private:
        Adafruit_TSL2591 sensor;
    public:
        SensorTSL2591();
        bool begin(const uint8_t ,const uint8_t);
        void registerInterrupt();
        void clearInterrupt();
        bool isOutsideBounds();
};