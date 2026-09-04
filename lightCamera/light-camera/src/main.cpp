#include <Arduino.h>
#include <led/led.hpp>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_LED_PIN 33
  LED led(DEBUG_LED_PIN);
#endif /* DEBUG */

void setup() 
{
  #ifdef DEBUG
  //Serial
  led.turnOn();
  #endif /* DEBUG */
}

void loop() 
{

}
