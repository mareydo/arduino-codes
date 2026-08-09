#include <Wire.h>
#include "Adafruit_TSL2591.h"
/////////////////////////////////////////////////////////////////

#define PIN_SDA 32
#define PIN_SCL 33
/////////////////////////////////////////////////////////////////
Adafruit_TSL2591 lightSensor = Adafruit_TSL2591(2591);
/////////////////////////////////////////////////////////////////
bool sensor_init()
{
  if(!Wire.begin(PIN_SDA,PIN_SCL))
  {
    Serial.println("No wire");
    return false;
  }
  else
  {
    Serial.println("Wire");
  }
  if(!lightSensor.begin(&Wire))
  {
    Serial.println("No sensor");
    return false;
  }
  else
  {
    Serial.println("Sensor");
  }
  return true;
}

void sensor_printLux()
{
  uint32_t raw = lightSensor.getLuminosity(TSL2591_FULLSPECTRUM);
  uint16_t lb = raw & 0xFFFF;
  uint16_t ub = raw >> 16;
  Serial.println(lightSensor.calculateLux(lb,ub));
}

#define VERSION 2
void setup() 
{
  delay(2000);
  Serial.begin(115200);
  Serial.println(VERSION);
  sensor_init();


}

void loop() 
{
  sensor_printLux();
  delay(500);

}
