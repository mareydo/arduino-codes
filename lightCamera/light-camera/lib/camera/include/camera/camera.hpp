#pragma once
#include <esp_camera.h>

#include <sd/sd.hpp>

class Camera
{
    private:
        bool takePicture();
        camera_fb_t* obtainBuffer();
        void releaseBuffer(camera_fb_t*);
    public:
        bool begin();
        bool begin(const camera_config_t&);
        void recordTimelapse(SD&);
};