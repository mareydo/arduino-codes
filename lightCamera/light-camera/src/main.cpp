#include <Arduino.h>
#include <led/led.hpp>
#include <sd/sd.hpp>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_LED_PIN 33
  LED led(DEBUG_LED_PIN);
#endif /* DEBUG */

SD sd;

void setup() 
{
  #ifdef DEBUG
  Serial.begin(115200);
  Serial.println("START");
  Serial.println(sd.isFull());
  led.turnOn();
  #endif /* DEBUG */
}

void loop() 
{

}
