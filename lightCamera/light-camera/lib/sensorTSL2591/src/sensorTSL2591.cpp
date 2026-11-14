#include <sensorTSL2591/sensorTSL2591.hpp>

#include <debug/debugPrint.h>

SensorTSL2591::SensorTSL2591(const uint8_t SDA,const uint8_t SCL)
                : pinSDA(SDA)
                , pinSCL(SCL)
                , sensor(2591) {}

bool SensorTSL2591::connect()
{
    if(!Wire.begin(pinSDA,pinSCL))
    {
        DEBUG_PRINTLN("No wire");
        return false;
    }
    else
    {
        DEBUG_PRINTLN("Wire begin");
    }
    if(!sensor.begin(&Wire))
    {
        DEBUG_PRINTLN("No sensor");
        return false;
    }
    else
    {
        DEBUG_PRINTLN("Sensor begin");
    }
    delay(100);
    return true;
}

void SensorTSL2591::disconnect()
{
    Wire.end();
}

// void sensor_printLux()
// {
//   uint32_t raw = sensor.getLuminosity(TSL2591_FULLSPECTRUM);
//   uint16_t lb = raw & 0xFFFF;
//   uint16_t ub = raw >> 16;
//   //Serial.println(sensor.calculateLux(lb,ub));
//   Serial.println(String(lb) + " " + String(ub));
// }

bool SensorTSL2591::registerInterrupt()
{
    if(!connect()) { return false; }
    sensor.registerInterrupt(SENSOR_LOLIM,
                                SENSOR_HILIM,
                                TSL2591_PERSIST_5);
    delay(300);
    sensor.enable();
    disconnect();
    return true;
}

bool SensorTSL2591::clearInterrupt()
{
    if(!connect()) { return false; }
    sensor.clearInterrupt();
    disconnect();
    return true;
}

bool SensorTSL2591::isOutsideBounds()
{
    // TODO
    if(!connect()) { return false; }
    uint32_t raw = sensor.getLuminosity(TSL2591_FULLSPECTRUM);
    uint16_t lb = raw & 0xFFFF;
    DEBUG_PRINTLN(lb);
    if((lb == 0xFFFF)||(lb == 0x0)) { return false; }
    disconnect();
    return lb <= SENSOR_LOLIM || lb >= SENSOR_HILIM;
    
}