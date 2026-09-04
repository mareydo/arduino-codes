#include "led/led.hpp"
#include "esp32-hal-gpio.h"
#include <Arduino.h>

LED::LED(const uint8_t pin)
{
    this->pin = pin;
    pinMode(pin, OUTPUT);
}
void LED::turnOn()
{
    digitalWrite(pin, LOW);
}
void LED::turnOff()
{
    digitalWrite(pin, HIGH);
}