#include <Wire.h>
#include <Adafruit_TSL2591.h>

Adafruit_TSL2591 lightSensor = Adafruit_TSL2591(2591);

bool sensor_init()
{
  if (!lightSensor.begin()) 
  { 
    Serial.println("no sensor"); 
    return false;
  }

  lightSensor.setGain(TSL2591_GAIN_MED);
  lightSensor.setTiming(TSL2591_INTEGRATIONTIME_100MS);
}

void sensor_printLux()
{
  uint32_t lum = lightSensor.getFullLuminosity();
  Serial.println(lightSensor.calculateLux(lum & 0xFFFF, lum >> 16));
}

void setup() {
  Serial.begin(9600);
  sensor_init();
  sensor_printLux();

}

void loop() {
  sensor_printLux();
  delay(1000);

}
