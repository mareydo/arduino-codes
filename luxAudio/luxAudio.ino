#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
///////////////////////////////////////////////////////
#define POT_PIN A0
///////////////////////////////////////////////////////
#define SELECT_BUTTON_PIN 2
#define APPLY_BUTTON_PIN 3
bool selectButtonPressed = false;
bool applyButtonPressed = false;
///////////////////////////////////////////////////////
#define NUMBER_OF_PLAYERS 3
#define DEFAULT_VOLUME 1 //0-30
byte mp3Index = 0;
DFRobotDFPlayerMini players[NUMBER_OF_PLAYERS];
byte volumes[NUMBER_OF_PLAYERS] = {DEFAULT_VOLUME, DEFAULT_VOLUME, DEFAULT_VOLUME};
///////////////////////////////////////////////////////
#define OLED_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
///////////////////////////////////////////////////////
void initOled()
{
  // initialize OLED display with address 0x3C for 128x64
  while (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("OLED init failed"));
  }

  delay(2000); // wait for initializing
  oled.clearDisplay();  
  oled.setTextColor(WHITE);
  oled.setTextSize(1);   
  oled.display();
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
  oled.setCursor(0,10);
  color = mp3Index == 0 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");

  oled.setCursor(0,20);
  color = mp3Index == 1 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");

  oled.setCursor(0,30);
  color = mp3Index == 2 ? WHITE : BLACK;  
  oled.setTextColor(color);
  oled.println("o");

  oled.setTextColor(WHITE);
  oled.display();
}
void printVolumes()
{
  
  oled.setCursor(60,10);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[0] < 10) { oled.print("0"); }
  oled.print(volumes[0]);
  oled.display();

  oled.setCursor(60,20);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[1] < 10) { oled.print("0"); }
  oled.print(volumes[1]);
  oled.display();
  
  oled.setCursor(60,30);
  oled.setTextColor(WHITE, BLACK);
  if(volumes[2] < 10) { oled.print("0"); }
  oled.print(volumes[2]);
  oled.display();
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
  bool isDown = digitalRead(SELECT_BUTTON_PIN) == LOW;
  delay(50);
  bool isDown2 = digitalRead(SELECT_BUTTON_PIN) == LOW;
  if(isDown != isDown2) return false;

  if(isDown != selectButtonPressed)
  {
    selectButtonPressed = isDown;
    return selectButtonPressed;
  }
  return false;
}
bool readApplyButtonDown()
{
  bool isDown = digitalRead(APPLY_BUTTON_PIN) == LOW;
  delay(50);
  bool isDown2 = digitalRead(APPLY_BUTTON_PIN) == LOW;
  if(isDown != isDown2) return false;

  if(isDown != applyButtonPressed)
  {
    applyButtonPressed = isDown;
    return applyButtonPressed;
  }
  return false;
}
///////////////////////////////////////////////////////
void initMp3Serial()
{
  ///////////////////////////////////////////////////////
  while (!players[0].begin(Serial1, true, true)) 
  {
    Serial.println("Mp3 0 Cannot Connect");
    delay(500);
  }
  Serial.println("Mp3 0 Connected");
  players[0].volume(DEFAULT_VOLUME);
  players[0].EQ(DFPLAYER_EQ_NORMAL);
  players[0].enableLoopAll();
//  myDFPlayer.EQ(DFPLAYER_EQ_POP);
//  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
//  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
//  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
//  myDFPlayer.EQ(DFPLAYER_EQ_BASS);
  ///////////////////////////////////////////////////////
  while (!players[1].begin(Serial2, true, true)) 
  {
    Serial.println("Mp3 1 Cannot Connect");
    delay(500);
  }
  Serial.println("Mp3 1 Connected");
  players[1].volume(DEFAULT_VOLUME);
  players[1].EQ(DFPLAYER_EQ_NORMAL);
  players[1].enableLoopAll();
  ///////////////////////////////////////////////////////
  while (!players[2].begin(Serial3, true, true)) 
  {
    Serial.println("Mp3 2 Cannot Connect");
    delay(500);
  }
  Serial.println("Mp3 2 Connected");
  players[2].volume(DEFAULT_VOLUME);
  players[2].EQ(DFPLAYER_EQ_NORMAL);
  players[2].enableLoopAll();
  ///////////////////////////////////////////////////////
}
void playMp3(byte i)
{
  players[i].next();
}
void stopMp3(byte i)
{
  players[i].stop();
}

void setVolume(byte index, byte volume)
{
  players[index].volume(volume);
  volumes[index] = volume;
}
void printMp3Detail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}
///////////////////////////////////////////////////////
#define ULTRASONIC_COUNT 6
#define TRIGGER_PAUSE_MS 500
#define TRIGGER_DISTANCE_CM 50
byte ultasonicTriggerPins[ULTRASOUND_COUNT];
byte ultasonicEchoPins[ULTRASOUND_COUNT];
byte ultrasonicIndex = 0;
void initUltrasonic()
{
  ultasonicTriggerPins[0] = 0;
  ultasonicTriggerPins[1] = 0;
  ultasonicTriggerPins[2] = 0;
  ultasonicTriggerPins[3] = 0;
  ultasonicTriggerPins[4] = 0;
  ultasonicTriggerPins[5] = 0;
  ultasonicTriggerPins[6] = 0;

  ultasonicEchoPins[0] = 0;
  ultasonicEchoPins[1] = 0;
  ultasonicEchoPins[2] = 0;
  ultasonicEchoPins[3] = 0;
  ultasonicEchoPins[4] = 0;
  ultasonicEchoPins[5] = 0;
  ultasonicEchoPins[6] = 0;

  for(byte i = 0; i <ULTRASONIC_COUNT; ++i)
  {
    pinMode(ultasonicTriggerPins[i],OUTPUT);
    pinMode(ultasonicEchoPins[i],INPUT);
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
  double duration_us = pulseIn(echoPin, HIGH);
  double distance_cm = 0.017 * duration_us;

  return distance_cm

}

bool isPresenceDetected(byte index)
{
  bool presence[3] = {false, false, false};
  for(byte i = 0; i < 3; ++i)
  {
    double distance = meassureDistanceCm(ultrasonicTriggerPins[index],ultrasonicEchoPins[index]);
    presence[i] = distance >= TRIGGER_DISTANCE_CM;
    sleep(300);
  }
  return presence[0] && presence[1] && presence[2]; 
}
///////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Mp3 Controller Start"));

  initUltrasonic();

  initOled();
  printStatic();
  printSelectedDot();
  printVolumes();

  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  initMp3Serial();
  //playMp3();

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
  if(isPresenceDetected(ultrasonicIndex))
  {
    Serial.print("Presence detected at: ");
    Serial.println(ultrasonicIndex/2);
    playMp3(ultrasonicIndex/2);
  }
  else
  {
    Serial.print("Presence not detected at: ");
    Serial.println(ultrasonicIndex/2);
    stopMp3(ultrasonicIndex/2);
  }
}