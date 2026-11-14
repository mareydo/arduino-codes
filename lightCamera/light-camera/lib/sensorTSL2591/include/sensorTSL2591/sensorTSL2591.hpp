#pragma once


#include <Adafruit_TSL2591/Adafruit_TSL2591.h>
#include <cstdint>

class SensorTSL2591
{
    private:
        uint8_t pinSDA;
        uint8_t pinSCL;

        Adafruit_TSL2591 sensor;

        bool connect();
        void disconnect();
    public:
        explicit SensorTSL2591(const uint8_t ,const uint8_t);
        SensorTSL2591(const SensorTSL2591& ) = delete;
        SensorTSL2591& operator=(const SensorTSL2591&) = delete;

        bool registerInterrupt();
        bool clearInterrupt();
        bool isOutsideBounds();
};