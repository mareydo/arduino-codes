#include "WString.h"
#include "esp32-hal.h"
#include <Arduino.h>

#include <config/default.cfg.h>
#include <debug/debugPrint.h>
#include <led/led.hpp>
#include <sd/sd.hpp>
#include <camera/camera.hpp>
#include <sensorTSL2591/sensorTSL2591.hpp>
/////////////////////////////////////////////////////////////////
RTC_DATA_ATTR bool inited = false;

uint8_t errorCode = 0;
#define ERROR_NONE  0b00
#define ERROR_SLEEP 0b01
#define ERROR_TIMER 0b10

#define CLEAR_ERROR errorCode = 0

#define SET_SLEEP_ERROR errorCode |= ERROR_SLEEP
#define SET_TIMER_ERROR errorCode |= ERROR_TIMER


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
  DEBUG_PRINTLN("Awake. Disabling interrupt");
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}
void esp_sleep()
{
  // save usr objects
  sensor.clearInterrupt();
  sd.storeDirName();
  // sleep mcu
  DEBUG_PRINTLN("Going to sleep. Enabling interrupt");
  gpio_pullup_en(PIN_INT);
  esp_sleep_enable_ext0_wakeup(PIN_INT, LOW);
  esp_deep_sleep_start();
}
void esp_timer_sleep()
{
  //TODO
  DEBUG_PRINTLN("Going to timer sleep");
}
/////////////////////////////////////////////////////////////////

//init sensor

//init camera
bool camera_init()
{
  if(camera.begin())
  {
    DEBUG_PRINTLN("Camera Inited");
    return true;
  }

  DEBUG_PRINTLN("Camera not Initialized");
  DEBUG_LED(led, HIGH);

  return false;
}

//init sd
bool sd_init()
{
  if(sd.loadDirName())
  {
    DEBUG_PRINTLN("SD Inited");
    return true;
  }

  DEBUG_PRINTLN("SD Dir Name not Initialized");
  DEBUG_LED(led, HIGH);

  return false;
}
/////////////////////////////////////////////////////////////////
void setup()
{
  // esp inter init
  delay(2000);
  bool retVal = false;

  #if DEBUG == 1
    Serial.begin(115200);
  #endif /* DEBUG */

  DEBUG_PRINTLN("Start " + String(digitalRead(PIN_INT)));
  if(inited)
  {
    //disable interrupt
    DEBUG_PRINTLN("INITED WAKEUP");
    esp_awake();
  }
  else
  {
    // DEBUG_PRINTLN("NOT INITED WAKEUP");
    // CLEAR_ERROR;
    // // setup one time info
    // // sensor interrupt
    // retVal = sensor.registerInterrupt();
    // DEBUG_PRINTLN("SENSOR UNPLUG WAIT");
    // delay(5000);
    // DEBUG_PRINTLN("WAIT ENDS");
    // if(!retVal) { SET_TIMER_ERROR; }
  }
  // camera
  retVal = camera_init();
  if(!retVal) { SET_SLEEP_ERROR; }

  DEBUG_PRINTLN("Before Begin" + String(digitalRead(PIN_INT)));
  //load run info
  retVal = sd_init();
  if(!retVal) { SET_SLEEP_ERROR; }
  //DEBUG_PRINTLN("After Begin" + String(digitalRead(PIN_INT)));
  //esp_initINT();
  //DEBUG_PRINTLN("After INT" + String(digitalRead(PIN_INT)));
  //sensor.clearInterrupt();
  DEBUG_PRINTLN("Setup finished");
}
bool loopMe = true;
void loop()
{
  /*if(errorCode & ERROR_TIMER)
  {
    DEBUG_PRINTLN("ERROR TIMER");
    esp_timer_sleep();
  }
  if(errorCode & ERROR_SLEEP)
  {
    DEBUG_PRINTLN("ERROR SLEEP");
    esp_sleep();
  }
  */
  //if(sensor.isOutsideBounds())
  if(loopMe)
  {
    DEBUG_PRINTLN("[foo]: Start " + String(digitalRead(PIN_INT)));
    sensor.isOutsideBounds();
    
    loopMe = false;
    // DEBUG_PRINTLN("IS OUTSIDE BOUNDS");
    // DEBUG_LED(led, HIGH); 
    DEBUG_PRINTLN("[foo]: before clear" + String(digitalRead(PIN_INT)));
    sensor.disable();
    DEBUG_PRINTLN("[foo]: after clear" + String(digitalRead(PIN_INT)));
    camera.recordTimelapse(sd);
   DEBUG_PRINTLN("[foo]: after record" + String(digitalRead(PIN_INT)));
  }
  // else
  // {
  //   //DEBUG_PRINTLN("IS INSIDE BOUNDS");
  //   //DEBUG_LED(led, LOW);
  //   //esp_sleep();
  // }
}
