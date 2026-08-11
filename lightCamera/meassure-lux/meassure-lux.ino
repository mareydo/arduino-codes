#include <Wire.h>
#include "Adafruit_TSL2591.h"
/////////////////////////////////////////////////////////////////

#define PIN_SDA 32
#define PIN_SCL 33
#define PIN_INT GPIO_NUM_25
RTC_DATA_ATTR bool err = false;
/////////////////////////////////////////////////////////////////
Adafruit_TSL2591 lightSensor = Adafruit_TSL2591(2591);
bool sensor_init()
{
  if(!Wire.begin(PIN_SDA,PIN_SCL))
  {
    Serial.println("No wire");
    err=true;
    return false;
  }
  else
  {
    Serial.println("Wire begin");
  }
  if(!lightSensor.begin(&Wire))
  {
    Serial.println("No sensor");
    err=true;
    return false;
  }
  else
  {
    Serial.println("Sensor begin");
  }
  delay(100);
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

#define SENSOR_HILIM 900
#define SENSOR_LOLIM 0

void sensor_registerInterrupt()
{
  lightSensor.registerInterrupt(SENSOR_LOLIM,
                                SENSOR_HILIM,
                                TSL2591_PERSIST_5);
  delay(300);
  lightSensor.enable();
}

bool sensor_isOutsideBounds()
{
  sensor_printLux();
  uint32_t raw = lightSensor.getLuminosity(TSL2591_FULLSPECTRUM);
  uint16_t lb = raw & 0xFFFF;
  if((lb == 0xFFFF)||(lb == 0x0)) 
  { 
    err=true;
    return false; 
  }
  return lb <= SENSOR_LOLIM || lb >= SENSOR_HILIM;
}
/////////////////////////////////////////////////////////////////
void esp_initINT()
{
  pinMode(PIN_INT, INPUT_PULLUP);
}

void esp_awake()
{
  Serial.println("Awake");
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

void esp_sleep()
{
  Serial.println("Going to sleep");
  gpio_pullup_en(PIN_INT); 
  esp_sleep_enable_ext0_wakeup(PIN_INT, LOW);
  esp_deep_sleep_start();
}
/////////////////////////////////////////////////////////////////
void camera_record()
{
  for(byte i = 0; i < 5; ++i) 
  {
    Serial.print(String(i) + ",");
    delay(1000);
  }
  Serial.println("");
}
/////////////////////////////////////////////////////////////////
RTC_DATA_ATTR bool inited = false;
void setup() 
{
  //esp32 is sometimes lazy
  delay(2000);
  Serial.begin(115200);

  if(inited)
  { 
    esp_awake();
  }
  if(!sensor_init()) 
  { 
    //turn on led, only for CAM
    esp_sleep();
    inited=false;
    err=true;
  }
  else
  {
    err=false;
  }
  sensor_registerInterrupt();
  if(!inited)
  {
    esp_initINT();  
    sensor_printLux();
    inited=true;
  }
}

void loop() 
{
  camera_record();
  if((err)||(!sensor_isOutsideBounds()))
  {
    lightSensor.clearInterrupt();
    esp_sleep();
  }
  
}
