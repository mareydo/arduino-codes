#include <EEPROM.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>

#include "debug/debugPrint.h"
#include "sd/sd.hpp"

SD::SD():actualDirName(-1), isConnected(false) {}

bool SD::loadDirName()
{
    EEPROM.begin(1);
    actualDirName = EEPROM.read(0);
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
    if(isConnected)
    {
        return true;
    }
    isConnected = false;

    if (!SD_MMC.begin("/sdcard", true))
    { // true enables 1-bit mode to free up GPIOs
        DEBUG_PRINTLN("SD Card Mount Failed")
        return false;
    }
    DEBUG_PRINTLN("SD Connected")

    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE)
    {
        DEBUG_PRINTLN("No SD Card attached")
        return false;
    }

    DEBUG_PRINTLN("SD Loaded")
    isConnected = true;
    return true;
}
void SD::disconnect()
{
    if(!isConnected)
    {
        return;
    }
    DEBUG_PRINTLN("SD Disconnected")
    SD_MMC.end();
    isConnected = false;
}

bool SD::createFolder()
{
    ++actualDirName;
    fs::FS &fs = SD_MMC;
    if(!fs.mkdir("/" + String(actualDirName)))
    {
        DEBUG_PRINTLN("Cannot create directory")
        return false;
    }

    DEBUG_PRINTLN("Created directory: /" + String(actualDirName));
    return true;
}
bool SD::removeOldest()
{
    uint16_t dirNumber = 0;
    fs::FS &fs = SD_MMC;
    while(dirNumber <= actualDirName)
    {

        DEBUG_PRINTLN("Checking" + String(dirNumber))
        DEBUG_PRINTLN(fs.exists("/" + String(dirNumber)))

        if(fs.exists("/"+String(dirNumber)))
        {

            DEBUG_PRINTLN("Removing " + String(dirNumber))

            fs.rmdir(String(dirNumber).c_str());
            return true;
        }
        ++ dirNumber;
    }

    DEBUG_PRINTLN("Cannot find Oldest folder")
    return false;
}
//TODO: can be deleted
bool SD::isFull()
{
    if(!connect()) { return false; }

    DEBUG_PRINT("Free space: ")
    DEBUG_PRINTLN(SD_MMC.totalBytes() - SD_MMC.usedBytes())

    bool retVal = SD_MMC.totalBytes() - SD_MMC.usedBytes() < REQUIRED_BYTES;
    disconnect();
    return retVal;
}
bool SD::writeToFile(const camera_fb_t* buffer, const uint16_t frameNumber)
{
    if(buffer == nullptr)
    {

        DEBUG_PRINTLN("Write to file camera buffer is null")
        return false;
    }
    if(!connect()) { return false; }
    fs::FS &fs = SD_MMC;
    if(frameNumber == 0)
    {
        if(!createFolder())
        {
            disconnect();
            return false;
        }
    }
    String fullPath = "/" + String(actualDirName) + "/" + String(frameNumber) + ".jpeg";
    File file = fs.open(fullPath.c_str(), FILE_WRITE);
    if(!file)
    {
        DEBUG_PRINTLN("Cannot open file " + fullPath)
        disconnect();
        return false;
    }
    if(SD_MMC.totalBytes() - SD_MMC.usedBytes() < buffer->len)
    {
        if(!removeOldest())
        {
            file.close();
            disconnect();
            return false;
        }
    }

    DEBUG_PRINTLN("Storing file " + fullPath)
    file.write(buffer->buf, buffer->len);
    file.flush();
    file.close();
    disconnect();
    return true; // TODO
}