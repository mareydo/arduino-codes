#pragma once

#include <config/default.cfg.h>

#if DEBUG == 1
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_LED(ledObject, value) (value) ? ledObject.turnOn(): ledObject.turnOff()
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_LED(ledObject, value)
#endif /* DEBUG */