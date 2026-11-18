#include <sensorTSL2591/sensorTSL2591.hpp>

#include <debug/debugPrint.h>

SensorTSL2591::SensorTSL2591(const uint8_t SDA,const uint8_t SCL)
                : pinSDA(SDA)
                , pinSCL(SCL)
                , sensor(2591) {}

bool SensorTSL2591::connect()
{
    if(isConnected) { return true; }
    isConnected = false;

    if(!Wire.begin(pinSDA,pinSCL))
    {
        DEBUG_PRINTLN("[Sensor]: No wire");
        return false;
    }
    DEBUG_PRINTLN("[Sensor]: Wire begin");

    // enables sensor
    if(!sensor.begin(&Wire))
    {
        DEBUG_PRINTLN("[Sensor]: No sensor");
        disconnect();
        return false;
    }
    isConnected = true;
    delay(500);
    DEBUG_PRINTLN("[Sensor]: Sensor connected");

    return true;
}

void SensorTSL2591::disconnect()
{
    
    if(!isConnected) { return; }
    Wire.end();
    isConnected = false;
    DEBUG_PRINTLN("[Sensor]: Sensor disconnected");
}

void SensorTSL2591::disable()
{
    DEBUG_PRINTLN("[Sensor]: Disable enter: " + String(digitalRead(13)));
    if(!connect()) { return; }
    DEBUG_PRINTLN("[Sensor]: After enter: " + String(digitalRead(13)));
    sensor.clearInterrupt();
    DEBUG_PRINTLN("[Sensor]: After clear: " + String(digitalRead(13)));
    sensor.disable();
    DEBUG_PRINTLN("[Sensor]: After disable: " + String(digitalRead(13)));
    disconnect();
    DEBUG_PRINTLN("[Sensor]: After disconnect: " + String(digitalRead(13)));
    DEBUG_PRINTLN("[Sensor]: Sensor disabled");
}

void SensorTSL2591::printLux()
{
    uint32_t raw = sensor.getLuminosity(TSL2591_FULLSPECTRUM);
    uint16_t lb = raw & 0xFFFF;
    uint16_t ub = raw >> 16;
    DEBUG_PRINTLN("[Sensor]: lux: " + String(sensor.calculateLux(lb,ub)));
    DEBUG_PRINTLN("[Sensor]: lb,ub:" +String(lb) + " " + String(ub));
}

bool SensorTSL2591::registerInterrupt()
{
    if(!connect()) { return false; }
    sensor.registerInterrupt(SENSOR_LOLIM,
                                SENSOR_HILIM,
                                TSL2591_PERSIST_5);
    delay(500);
    sensor.clearInterrupt();
    disconnect();
    DEBUG_PRINTLN("[Sensor]: Interrupt registered");
    return true;
}

bool SensorTSL2591::clearInterrupt()
{
    if(!connect()) { return false; }
    sensor.clearInterrupt();
    disconnect();
    DEBUG_PRINTLN("[Sensor]: Interrupt cleared");
    return true;
}

bool SensorTSL2591::isOutsideBounds()
{
    // TODO
    if(!connect()) { return false; }
    uint32_t raw = sensor.getLuminosity(TSL2591_FULLSPECTRUM);
    disconnect();

    uint16_t lb = raw & 0xFFFF;
    if((lb == 0xFFFF)||(lb == 0x0)) 
    {
        DEBUG_PRINTLN("[Sensor]: Invalid Bounds reading value");
        return false;
    }
    //DEBUG_PRINTLN(lb);
    DEBUG_PRINTLN("[Sensor]: Is outside bound " + String(lb <= SENSOR_LOLIM || lb >= SENSOR_HILIM));
    return lb <= SENSOR_LOLIM || lb >= SENSOR_HILIM;

}