#include <Arduino.h>

#include <config/default.cfg.h>
#include <debug/debugPrint.h>
#include <led/led.hpp>
#include <sd/sd.hpp>
#include <camera/camera.hpp>
#include <sensorTSL2591/sensorTSL2591.hpp>
/////////////////////////////////////////////////////////////////
RTC_DATA_ATTR bool inited = false;
bool noErr = true;

/////////////////////////////////////////////////////////////////
LED led(DEBUG_LED_PIN);
SD sd;
Camera camera;
SensorTSL2591 sensor(PIN_SDA, PIN_SCL);
/////////////////////////////////////////////////////////////////
void esp_initINT()
{
  pinMode(PIN_INT, INPUT_PULLUP);
}
void esp_awake()
{
  DEBUG_PRINTLN("Awake");
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}
void esp_sleep()
{
  // save usr objects
  sensor.clearInterrupt();
  noErr = true; // FIXME: not sure about this
  sd.storeDirName();
  // sleep mcu
  DEBUG_PRINTLN("Going to sleep");
  gpio_pullup_en(PIN_INT);
  esp_sleep_enable_ext0_wakeup(PIN_INT, LOW);
  esp_deep_sleep_start();
}
/////////////////////////////////////////////////////////////////

//init sensor
bool sensor_init()
{
  return true;/*
  if(sensor.begin()) { return true; }

  DEBUG_PRINTLN("Sensor not Initialized");
  DEBUG_LED(led, HIGH);
  esp_sleep();

  return false;*/
}
//init camera
bool camera_init()
{
  if(camera.begin()) { return true; }

  DEBUG_PRINTLN("Camera not Initialized");
  DEBUG_LED(led, HIGH);
  esp_sleep();

  return false;
}
//init sd
bool sd_init()
{
  if(sd.loadDirName()) { return true; }

  DEBUG_PRINTLN("SD Dir Name not Initialized");
  DEBUG_LED(led, HIGH);
  esp_sleep();

  return false;
}
/////////////////////////////////////////////////////////////////
void setup()
{
  // esp inter init
  delay(2000);
  noErr = true;
  #if DEBUG == 1
    Serial.begin(115200);
  #endif /* DEBUG */

  if(inited)
  {
    DEBUG_PRINTLN("INITED");
    //disable interrupt
    esp_awake();
  }
  DEBUG_PRINT("START: ");
  DEBUG_PRINTLN(noErr);
  DEBUG_PRINT("SENSOR: ");
  //init sensor
  noErr |= sensor_init();
  DEBUG_PRINTLN(noErr);
  //init camera
  DEBUG_PRINT("CAMERA: ");
  noErr |= camera_init();
  DEBUG_PRINTLN(noErr);
  //init sd
  DEBUG_PRINT("SD: ");
  noErr |= sd_init();
  DEBUG_PRINTLN(noErr);

  //register sensor interrupt
  noErr |= sensor.registerInterrupt();

  if(!inited)
  {
    esp_initINT();
    inited=true;
  }
  DEBUG_PRINTLN("Setup finished");
}

void loop() 
{
  camera.recordTimelapse(sd);
  if((!noErr) || (!sensor.isOutsideBounds()))
  {
    esp_sleep();
  }
}
