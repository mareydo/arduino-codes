#include "Arduino.h"
#include "EEPROM.h"
#include "DFRobotDFPlayerMini.h"

#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
///////////////////////////////////////////////////////
#define POT_PIN A0
///////////////////////////////////////////////////////
#define SELECT_BUTTON_PIN 7
#define APPLY_BUTTON_PIN 2
#define BUTTON_DEBOUNCE_MS 50
///////////////////////////////////////////////////////
#define NUMBER_OF_PLAYERS 1
#define DEFAULT_VOLUME 1 //0-30
byte mp3Index = 0;
DFRobotDFPlayerMini players[NUMBER_OF_PLAYERS];

byte volumes[NUMBER_OF_PLAYERS];
bool isPlaying[NUMBER_OF_PLAYERS] = {0};
///////////////////////////////////////////////////////
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
///////////////////////////////////////////////////////
#define ULTRASONIC_COUNT 6
#define TRIGGER_PAUSE_MS 10
#define TRIGGER_DISTANCE_CM 10
#define DETECTION_DEBOUNCE_MS 500

byte ultasonicTriggerPins[ULTRASONIC_COUNT];
byte ultasonicEchoPins[ULTRASONIC_COUNT];
byte ultrasonicIndex = 0;
byte presenceDetected[ULTRASONIC_COUNT];
///////////////////////////////////////////////////////
void initOled()
{
  Serial.println("OLED init Start");
  // initialize OLED display with address 0x3C for 128x64
  while (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
  }
  delay(2000); // wait for initializing
  oled.clearDisplay();  
  oled.setTextColor(WHITE);
  oled.setTextSize(1);   
  oled.display();
  Serial.println("OLED init done");
}
void printStatic()
{
  oled.setCursor(10,10);
  oled.println("Mp3 0");
  oled.setCursor(10,20);
  oled.println("Mp3 1");
  oled.setCursor(10,30);
  oled.println("Mp3 2");
  oled.display();
}
void printSelectedDot()
{
  uint16_t color;
  #if NUMBER_OF_PLAYERS > 0
  oled.setCursor(0,10);
  color = mp3Index == 0 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");
  #endif

  #if NUMBER_OF_PLAYERS > 1
  oled.setCursor(0,20);
  color = mp3Index == 1 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");
  #endif 

  #if NUMBER_OF_PLAYERS > 2
  oled.setCursor(0,30);
  color = mp3Index == 2 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");
  #endif

  oled.setTextColor(WHITE);
  oled.display();
}
void printVolumes()
{
  #if NUMBER_OF_PLAYERS > 0
  oled.setCursor(60,10);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[0] < 10) { oled.print("0"); }
  oled.print(volumes[0]);
  oled.display();
  #endif

  #if NUMBER_OF_PLAYERS > 1
  oled.setCursor(60,20);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[1] < 10) { oled.print("0"); }
  oled.print(volumes[1]);
  oled.display();
  #endif
  
  #if NUMBER_OF_PLAYERS > 2
  oled.setCursor(60,30);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[2] < 10) { oled.print("0"); }
  oled.print(volumes[2]);
  oled.display();
  #endif
}
void printActualValue(byte value)
{
  oled.setTextSize(2);  
  oled.setCursor(10,50); 
  oled.setTextColor(WHITE, BLACK);
  if(value < 10) { oled.print("0"); }
  oled.print(value);
  oled.setTextSize(1);   

  oled.display();
}
///////////////////////////////////////////////////////
byte readPot()
{
  int raw = analogRead(POT_PIN);
  if(raw < 20) { raw = 20; }
  if(raw > 1000) { raw = 1000; }
  return map(raw, 20, 1000, 0, 30);
}
///////////////////////////////////////////////////////
bool readSelectButtonDown()
{
  static unsigned long lastDebounceTime = 0;  
  static bool lastButtonState = HIGH;
  bool retVal = false;
  bool reading = digitalRead(SELECT_BUTTON_PIN);
  // edge
  if (reading != lastButtonState) 
  {
    lastDebounceTime = millis();
  }
  // hold
  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) 
  {
    if(reading == LOW)
    {
      retVal = true;
    }
    lastDebounceTime = millis();
  }
  lastButtonState = reading;
  return retVal;
}
bool readApplyButtonDown()
{
  static unsigned long lastDebounceTime = 0;  
  static bool lastButtonState = HIGH;
  bool retVal = false;
  bool reading = digitalRead(APPLY_BUTTON_PIN);
  // edge
  if (reading != lastButtonState) 
  {
    lastDebounceTime = millis();
  }
  // hold
  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) 
  {
    if(reading == LOW)
    {
      retVal = true;
    }
    lastDebounceTime = millis();
  }
  lastButtonState = reading;
  return retVal;
}
///////////////////////////////////////////////////////
void initMp3Serial()
{
  byte storedVolume = DEFAULT_VOLUME;

  HardwareSerial* dfSerial;
  for(byte i=0; i < NUMBER_OF_PLAYERS; ++i)
  {
    if(i==0) dfSerial = &Serial1;
    if(i==1) dfSerial = &Serial2;
    if(i==2) dfSerial = &Serial3;
    while (!players[i].begin(*dfSerial, true, true)) 
    {
      Serial.println("Mp3 " + String(i) + " Cannot Connect");
      oled.setCursor(10,40);
      oled.setTextColor(WHITE, BLACK);
      oled.println("MP3" + String(i) + " ERR");
      delay(500);
    }
     Serial.println("Mp3 0 Connected");
    storedVolume = EEPROM.read(0);
    volumes[i] = storedVolume;
    players[i].volume(storedVolume);
    players[i].EQ(DFPLAYER_EQ_NORMAL);
    players[i].stop();
    players[i].enableLoopAll();
  }
}
void playMp3(byte i)
{
  if(isPlaying[i]) return;
  isPlaying[i] = true;
  players[i].start();
}
void stopMp3(byte i)
{
  if(!isPlaying[i]) return;
  isPlaying[i] = false;
  players[i].stop();
}
void setVolume(byte index, byte volume)
{
  EEPROM.update(index, volume);
  players[index].volume(volume);
  volumes[index] = volume;
}
///////////////////////////////////////////////////////

void initUltrasonic()
{
  #if NUMBER_OF_PLAYERS > 0
  ultasonicTriggerPins[0] = 23;
  ultasonicTriggerPins[1] = 27;
  # endif

  #if NUMBER_OF_PLAYERS > 1
  ultasonicTriggerPins[2] = 31;
  ultasonicTriggerPins[3] = 35;
  #endif

  #if NUMBER_OF_PLAYERS > 2
  ultasonicTriggerPins[4] = 39;
  ultasonicTriggerPins[5] = 43;
  #endif

  #if NUMBER_OF_PLAYERS > 2
  ultasonicEchoPins[0] = 25;
  ultasonicEchoPins[1] = 29;
  #endif
  
  #if NUMBER_OF_PLAYERS > 2
  ultasonicEchoPins[2] = 33;
  ultasonicEchoPins[3] = 37;
  #endif
  
  #if NUMBER_OF_PLAYERS > 2
  ultasonicEchoPins[4] = 41;
  ultasonicEchoPins[5] = 45;
  #endif


  for(byte i = 0; i <ULTRASONIC_COUNT; ++i)
  {
    pinMode(ultasonicTriggerPins[i],OUTPUT);
    pinMode(ultasonicEchoPins[i],INPUT);
    presenceDetected[i] = false;
  }
}

void generateTrigger(byte pin)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
}

double meassureDistanceCm(byte trgPin, byte echoPin)
{
  generateTrigger(trgPin);
  double duration_us = pulseIn(echoPin, HIGH, 1000);
  duration_us = duration_us == 0 ? 999 : duration_us;
  double distance_cm = 0.017 * duration_us;

  return distance_cm;

}
void detectPresence(byte index)
{
  static unsigned long lastChangeTimes[ULTRASONIC_COUNT] = { 0 };
  static bool lastReadings[ULTRASONIC_COUNT] = { 0 };

  bool reading = meassureDistanceCm(ultasonicTriggerPins[index], ultasonicEchoPins[index]) < TRIGGER_DISTANCE_CM;

  if (reading != lastReadings[index])
  {
    lastReadings[index] = reading;
    lastChangeTimes[index] = millis();
  }

  if ((millis() - lastChangeTimes[index]) >= DETECTION_DEBOUNCE_MS)
  {
    if (presenceDetected[index] != reading)
    {
      presenceDetected[index] = reading;
      printPresence();   // print only on confirmed change
    }
  }
}

void printPresence()
{
  uint16_t color;
  for(byte i = 0; i < ULTRASONIC_COUNT; ++i)
  {
    color = presenceDetected[i] ? WHITE : BLACK;  
    oled.setCursor(100,10*i);
    oled.setTextColor(color);
    oled.print(i);
    oled.display();

  }
}
///////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println("Mp3 Controller Start");

  initUltrasonic();

  initOled();
  printStatic();
  printSelectedDot();

  #if NUMBER_OF_PLAYERS > 0
  Serial1.begin(9600);
  #endif
  #if NUMBER_OF_PLAYERS > 1
  Serial2.begin(9600);
  #endif
  #if NUMBER_OF_PLAYERS > 2
  Serial3.begin(9600);
  #endif
  initMp3Serial();
  //playMp3();
  printVolumes();

  pinMode(APPLY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(POT_PIN, INPUT);
}
void loop()
{
  if(readApplyButtonDown())
  {
    setVolume(mp3Index,readPot());
    printVolumes();
  }
  else if(readSelectButtonDown())
  {
    ++mp3Index;
    if(mp3Index >= NUMBER_OF_PLAYERS) { mp3Index = 0; }
    
    printSelectedDot();
  } 
  else
  {    
    printActualValue(readPot());
  }
  ++ultrasonicIndex;
  if(ultrasonicIndex == ULTRASONIC_COUNT)
  {
    ultrasonicIndex = 0;
  }
  byte mp3ConvertedIndex = ultrasonicIndex/2; 
  detectPresence(ultrasonicIndex);

  if(presenceDetected[mp3ConvertedIndex*2] || presenceDetected[mp3ConvertedIndex*2+1])
  {
    playMp3(mp3ConvertedIndex);
  }

  if(!presenceDetected[mp3ConvertedIndex*2] && !presenceDetected[mp3ConvertedIndex*2+1])
  {
    stopMp3(mp3ConvertedIndex);
  }

}