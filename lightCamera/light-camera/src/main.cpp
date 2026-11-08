#include <Arduino.h>

#include <config/default.cfg.h>
#include <debug/debugPrint.h>
#include <led/led.hpp>
#include <sd/sd.hpp>
#include <camera/camera.hpp>
#include <sensorTSL2591/sensorTSL2591.hpp>
/////////////////////////////////////////////////////////////////
RTC_DATA_ATTR bool inited = false;
bool err = false;

/////////////////////////////////////////////////////////////////
LED led(DEBUG_LED_PIN);
SD sd;
Camera camera;
SensorTSL2591 sensor;
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
  err = false; // FIXME: not sure about this
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
  if(sensor.begin(PIN_SDA, PIN_SCL)) { return true; }

  DEBUG_PRINTLN("Sensor not Initialized");
  DEBUG_LED(led, HIGH);
  esp_sleep();

  return false;
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
  err = false;
  #if DEBUG == 1
    Serial.begin(115200);
    DEBUG_PRINTLN("START");
  #endif /* DEBUG */

  if(inited)
  {
    //disable interrupt
    esp_awake();
  }

  //init sensor
  //err |= sensor_init();
  //init camera
  err |= camera_init();
  //init sd
  err |= sd_init();

  //register sensor interrupt
  sensor.registerInterrupt();

  if(!inited)
  {
    esp_initINT();
    inited=true;
  }
}

void loop() 
{
  camera.recordTimelapse(sd);
  //if((err) || (!sensor.isOutsideBounds()))
  if(err)
  {
    esp_sleep();
  }
}
