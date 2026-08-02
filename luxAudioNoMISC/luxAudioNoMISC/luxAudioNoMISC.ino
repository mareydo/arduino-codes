#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

//#include "Wire.h"
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
#define NUMBER_OF_PLAYERS 2
DFRobotDFPlayerMini players[NUMBER_OF_PLAYERS];
byte volumes[NUMBER_OF_PLAYERS] = {25, 25};
byte isPlaying[NUMBER_OF_PLAYERS] = {false, false};
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
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
  players[0].volume(volumes[0]);
  players[0].EQ(DFPLAYER_EQ_NORMAL);
  players[0].enableLoopAll();
  players[0].start();
  ///////////////////////////////////////////////////////
  while (!players[1].begin(Serial2, true, true)) 
  {
    Serial.println("Mp3 1 Cannot Connect");
    delay(500);
  }
  Serial.println("Mp3 1 Connected");   
  players[1].volume(volumes[1]);
  players[1].EQ(DFPLAYER_EQ_NORMAL);
  players[1].enableLoopAll();
  players[1].start();
  ///////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println("Mp3 Controller Start");

  Serial1.begin(9600);
  Serial2.begin(9600);
  initMp3Serial();
  Serial.println("Mp3 Controller Inited");
}
void loop()
{}