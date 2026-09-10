#pragma once
#include <cstdint>
#include <esp_camera.h>

#define VIDEO_FRAME_COUNT 5
#define VIDEO_FPS_RATE 1 //10fps
#define FPS_2_MS(ms) 1000.0/ms
#define REQUIRED_BYTES VIDEO_FRAME_COUNT*25*1024 //TODO find better max jpeg size
class SD
{
    private:
        uint16_t actualDirName;

        bool createFolder();
        bool removeOldest();

        bool connect();
        void disconnect();
    public:
        bool isFull();

        bool loadDirName();
        bool storeDirName();
        bool writeToFile(const camera_fb_t*);
};