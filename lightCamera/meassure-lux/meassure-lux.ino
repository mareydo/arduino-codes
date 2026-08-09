#include <Wire.h>
#include "Adafruit_TSL2591.h"
/////////////////////////////////////////////////////////////////

#define PIN_SDA 32
#define PIN_SCL 33
#define PIN_INT 25
/////////////////////////////////////////////////////////////////
Adafruit_TSL2591 lightSensor = Adafruit_TSL2591(2591);
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
  //Serial.println(lightSensor.calculateLux(lb,ub));
  Serial.println(String(lb) + " " + String(ub));
}

#define SENSOR_HILIM 1500
#define SENSOR_LOLIM 100

void sensor_registerInterrupt()
{
  lightSensor.registerInterrupt(SENSOR_LOLIM,
                                SENSOR_HILIM,
                                TSL2591_PERSIST_5);
  delay(300);
  lightSensor.enable();
}
/////////////////////////////////////////////////////////////////
void esp_initINT()
{
  pinMode(PIN_INT, INPUT_PULLUP);
}
/////////////////////////////////////////////////////////////////
void setup() 
{
  delay(2000);
  Serial.begin(115200);
  sensor_init();
  esp_initINT();
  sensor_registerInterrupt();

}

void loop() 
{
  //sensor_printLux();
  if(digitalRead(PIN_INT)==LOW)
  {
    sensor_printLux();
    lightSensor.clearInterrupt();
  }
  delay(500);

}
