#pragma once
#include <cstdint>

class LED
{
    private:
        uint8_t pin;
    public:
        LED(const uint8_t);
        void turnOn();
        void turnOff();
};