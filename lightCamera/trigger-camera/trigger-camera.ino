#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <EEPROM.h>

#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35  
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21  //21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

//#define VIDEO_DURATION_MS 5000
#define VIDEO_FRAME_COUNT 5
#define VIDEO_FPS_RATE 1 //10fps
#define FPS_2_MS(ms) 1000.0/ms
#define REQUIRED_BYTES VIDEO_FRAME_COUNT*25*1024 //TODO find better max jpeg size


#define DEBUG_LED
#ifdef DEBUG_LED
  #define DEBUG_LED_PIN 33
#endif



bool SD_connect()
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

void SD_disconnect()
{
  SD_MMC.end();
}

esp_err_t camera_init()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  camera_config_t camera_config = {
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
    .grab_mode = CAMERA_GRAB_LATEST//CAMERA_GRAB_LATEST. Sets when buffers should be filled
  };


  if(CAM_PIN_PWDN != -1){
      pinMode(CAM_PIN_PWDN, OUTPUT);
      digitalWrite(CAM_PIN_PWDN, LOW);
  }

  //initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) 
  {
    Serial.print("Init ended with error ");
    Serial.println(err);
    return err;
  }

  #ifdef DEBUG_LED
    pinMode(DEBUG_LED_PIN,OUTPUT);
  #endif

  delay(1000);
  return ESP_OK;
}

void camera_turnLEDOn() {
  #ifdef DEBUG_LED
  digitalWrite(DEBUG_LED_PIN,LOW);
  #endif
}

void camera_turnLEDOff() {
  #ifdef DEBUG_LED
  digitalWrite(DEBUG_LED_PIN,HIGH);
  #endif
}

RTC_DATA_ATTR uint8_t videoNumber = 1;
void camera_loadVideoNumber()
{
  EEPROM.begin(1);
  videoNumber = EEPROM.read(0) + 1;
}

void camera_saveVideoNumber()
{
  EEPROM.write(0, videoNumber);
  EEPROM.commit();
}

esp_err_t camera_takePicture(const char* path)
{
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  fs::FS &fs = SD_MMC;
  File file = fs.open(path, FILE_WRITE);
  if(!file)
  { 
    Serial.print("No file ");
    Serial.println(path);
    return ESP_ERR_INVALID_ARG; 
  }
  fb = esp_camera_fb_get();
  if(!fb) 
  {
    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }
  file.write(fb->buf, fb->len);
  file.close();
  esp_camera_fb_return(fb);
  Serial.println("Picture taken");
  return ESP_OK;
}



void camera_timelapse() 
{

  if(!SD_createDir()) { return; }
  unsigned long lastTime = -1;
  unsigned long frameDuration = FPS_2_MS(VIDEO_FPS_RATE);

  unsigned long frameCounter = 0;
  while(1)
  {
    unsigned long now = millis();
    if(lastTime == -1) { lastTime = now; }
    unsigned long elapsed = now - lastTime;
    if(elapsed > frameDuration )
    {
      String fullPath = "/" + String(videoNumber) + "/" + String(frameCounter) + ".jpeg";
      if(camera_takePicture(fullPath.c_str()) != ESP_OK) { break; }
      ++frameCounter;
      lastTime = now;
    }
    if(frameCounter == VIDEO_FRAME_COUNT)
    {
      break;
    }
  }
  ++videoNumber;
  return;
}

void SD_removeOldestDir()
{
  uint16_t dirName = 0;
  fs::FS &fs = SD_MMC;
  while(dirName <= videoNumber)
  {
    Serial.println("Checking" + String(dirName));
    Serial.println(fs.exists("/" + String(dirName)));
    if(fs.exists("/"+String(dirName)))
    {
      Serial.print("Removing ");
      Serial.println(dirName);
      fs.rmdir(String(dirName).c_str());
      return;
    }
    ++ dirName;
  }
  Serial.println("Cannot find Oldest folder");
  return;
}

bool SD_createDir()
{
  fs::FS &fs = SD_MMC;
  if(!fs.mkdir("/" + String(videoNumber)))
  {
    Serial.println("Cannot create directory");
    return false;
  }
  return true;
}


bool SD_isFull()
{
  Serial.print("Used: ");
  Serial.println(SD_MMC.totalBytes() - SD_MMC.usedBytes());
  return SD_MMC.totalBytes() - SD_MMC.usedBytes() < REQUIRED_BYTES;
}

void setup() 
{
  delay(2000);
  Serial.begin(115200);
  Serial.println("Begin");
  //camera_loadVideoNumber();
  esp_err_t retVal = camera_init();
  bool ret = SD_connect();

  Serial.print("Inited with code ");
  Serial.println(String(retVal) + " " + String(ret));

  camera_turnLEDOn();
  Serial.println("START");
  Serial.print("Is card full? ");
  Serial.println(SD_isFull());
  camera_timelapse();
  Serial.println("STOP");
  camera_turnLEDOff();
  //camera_saveVideoNumber();
  SD_removeOldestDir();
  SD_disconnect();

}

void loop() {
  // put your main code here, to run repeatedly:

}
