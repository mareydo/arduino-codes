#include <Arduino.h>
#include <soc/soc.h>         // Disable brownout problems
#include <soc/rtc_cntl_reg.h>  // Disable brownout problems
#include <esp_camera.h>

#include <camera/camera.hpp>
#include <debug/debugPrint.h>


#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

#define VIDEO_FRAME_COUNT 5
#define VIDEO_FPS_RATE 1 //10fps
#define FPS_2_MS(ms) 1000.0/ms

bool Camera::begin()
{
    const camera_config_t config =
    {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_SVGA,//QQVGA-UXGA

        .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST,//CAMERA_GRAB_LATEST. Sets when buffers should be filled
    };
    return begin(config);
}

bool Camera::begin(const camera_config_t &config)
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    if(config.pin_pwdn != -1)
    {
        pinMode(CAM_PIN_PWDN, OUTPUT);
        digitalWrite(CAM_PIN_PWDN, LOW);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        DEBUG_PRINT("Init ended with error ")
        DEBUG_PRINTLN(err)

        return false;
    }

    // necessary for changes to take place
    delay(1000);
    return true;
}


camera_fb_t* Camera::obtainBuffer()
{
    camera_fb_t* fb;

    fb = esp_camera_fb_get();
    if(!fb)
    {
        DEBUG_PRINTLN("Camera capture failed")
        return nullptr;
    }
    return fb;
}

void Camera::releaseBuffer(camera_fb_t* fb)
{
    esp_camera_fb_return(fb);
}

void Camera::recordTimelapse(SD& sd)
{
    unsigned long frameDuration = FPS_2_MS(VIDEO_FPS_RATE);

    uint16_t frameCounter = 0;
    while(1)
    {
        unsigned long now = millis();
        static unsigned long lastTime = now;
        unsigned long elapsed = now - lastTime;

        if(elapsed > frameDuration )
        {
            camera_fb_t* fb = obtainBuffer();
            if(!fb)
            {
                DEBUG_PRINTLN("Failed to obtain buffer")
                return;
            }
            if(!sd.writeToFile(fb, frameCounter))
            {
                releaseBuffer(fb);
                break;
            }
            releaseBuffer(fb);
            DEBUG_PRINTLN("Made photo " + String(frameCounter))
            ++frameCounter;
            lastTime = now;
        }
        if(frameCounter >= VIDEO_FRAME_COUNT)
        {
            break;
        }
    }
    return;
}
