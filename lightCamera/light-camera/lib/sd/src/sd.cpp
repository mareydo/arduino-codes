#include <EEPROM.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>

#include "sd/sd.hpp"
bool SD::loadDirName()
{
    EEPROM.begin(1);
    actualDirName = EEPROM.read(0) + 1;
    return true; //TODO
}
bool SD::storeDirName()
{
    EEPROM.write(0, actualDirName);
    EEPROM.commit();
    return true; //TODO
}

bool SD::connect()
{
    if (!SD_MMC.begin("/sdcard", true))
    { // true enables 1-bit mode to free up GPIOs
        Serial.println("SD Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE)
    {
        Serial.println("No SD Card attached");
        return false;
    }
    return true;
}
void SD::disconnect()
{
    #ifdef DEBUG
        Serial.println("SD Disconnect");
    #endif
    SD_MMC.end();
}

bool SD::createFolder()
{
    if(!connect()) { return false; }
    fs::FS &fs = SD_MMC;
    if(!fs.mkdir("/" + String(actualDirName)))
    {
        #ifdef DEBUG
            Serial.println("Cannot create directory");
        #endif
        disconnect();
        return false;
    }
    disconnect();
    return true;
}
bool SD::removeOldest()
{
    if(!connect()) { return false; }
    uint16_t dirNumber = 0;
    fs::FS &fs = SD_MMC;
    while(dirNumber <= actualDirName)
    {
        #ifdef DEBUG
            Serial.println("Checking" + String(dirNumber));
            Serial.println(fs.exists("/" + String(dirNumber)));
        #endif
        if(fs.exists("/"+String(dirNumber)))
        {
            #ifdef DEBUG
                Serial.print("Removing ");
                Serial.println(dirNumber);
            #endif
            fs.rmdir(String(dirNumber).c_str());
            return true;
        }
        ++ dirNumber;
    }
    #ifdef DEBUG
        Serial.println("Cannot find Oldest folder");
    #endif
    return false;
}
bool SD::isFull()
{
    if(!connect()) { return false; }
    #ifdef DEBUG
        Serial.print("Used: ");
        Serial.println(SD_MMC.totalBytes() - SD_MMC.usedBytes());
    #endif
    return SD_MMC.totalBytes() - SD_MMC.usedBytes() < REQUIRED_BYTES;
}
bool SD::writeToFile(const camera_fb_t* buffer)
{
    if(!connect()) { return false; }
    fs::FS &fs = SD_MMC;
    File file = fs.open("/" + String(actualDirName), FILE_WRITE);
    ++actualDirName;
    disconnect();
    return true; // TODO
}