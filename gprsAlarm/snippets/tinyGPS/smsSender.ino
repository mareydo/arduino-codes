
#include <SoftwareSerial.h>
#include "TinyGPS++.h"
#include <stdio.h>

byte ERLED = 13;
byte PWRON = 8; //remote turning on the module
bool want = false;
#define BUFF_SIZE 500
char readStr[BUFF_SIZE];
int index=0;

SoftwareSerial SmsSerial(2,3); //rx,tx
TinyGPSPlus gps;

bool InitGeneral()
{ 
  //turn module on
  digitalWrite(PWRON,HIGH);
  delay(1000);
  digitalWrite(PWRON,LOW);
  delay(3000);
  //start init and give module time to connect
  Serial.println("INIT Starts");
  SmsSerial.begin(9600);
  delay(20000);

  
  Serial.println("while");
  int i;
  for(i = 0 ; i < 5 ; ++i)
  {
    if(sendToUART("AT","OK")) break;
    delay(200);
  }
  if(i==5)
  {
    while(SmsSerial.available()) SmsSerial.read();
    Serial.println("IT'S NOT CONNECTING :o ");
  }
  
  while(!(sendToUART("AT","OK"))){delay(100);}

  //show that init is done
  digitalWrite(ERLED,HIGH);
  delay(3000);
  digitalWrite(ERLED,LOW);

  //setup module txt mode
  if(!(sendToUART("AT+CMGF=1","OK"))) return false;
  Serial.println("General Init");
  delay(1000);
  Serial.flush();
  return true;
}

bool InitReciever()
{ 
  if(!(sendToUART("AT+CNMI=2,2,0,0,0",""))) return false;
  delay(3000);
  return true;
}

bool InitSender(const char* telNum)
{
  char setupString[30] = "AT+CMGS=\"";  //12chars + 18chars for number format
  strcat(setupString,telNum);
  strcat(setupString,"\""); 
  if(!(sendToUART(setupString,">"))) return false;
  delay(3000);
  return true;
}

bool InitGPS()
{
  sendToUART("AT+GPS=1","OK");
  delay(200);
  sendToUART("AT+GPSRD=1","OK");
  delay(5000);
  sendToUART("AT+GPSRD=0","");
}

bool SendSMS(const char* smsText)
{ 
  if(!(InitSender("+421907637282"))) ErrorLED(3);;
  delay(3000);
  if(!sendToUART(smsText,"")) return false;
  SmsSerial.write(26);  //ctrl+z end char
  delay(3000);
 // if(!(InitReciever())) ErrorLED(2);;
  return true;
}

//--------------------------------------------------------------------------------------------------
void setup()
{
  
  Serial.begin(9600);     //PC debug comunication

  /* A7 default br is 115200
   * do only next three commands to set it on 9600
   * settings will be saved, next serial comunication will be at 9600
   */
//  SmsSerial.begin(115200);
//  sendToUART("AT+IPR=9600","");
//  sendToUART("AT&WD","");
  
  pinMode(ERLED,OUTPUT);          //error output control
  pinMode(PWRON,OUTPUT);  
  
  /*InitSender is sendig commands immediately and controling them
   * WARNING modul after boot will send info output
   * there must be at least 3sec delay between first command and boot
   * commands could otherwise crash program by sending wrong output to UART
   */
    
  if(!(InitGeneral())) ErrorLED(1);
    
  if(!(SendSMS("INIT DONE"))) ErrorLED(3);
  InitGPS();
  
}
//--------------------------------------------------------------------------------------------------

void loop()
{
  if(checkForChar('g'))
  {
   Serial.println("GPS");
    SendSMS("Waiting for GPS");
    while(gps.satellites.value()<=0.0);
    SendSMS("HAVE SATELITE");
  }
    
     
}

//--------------------------------------------------------------------------------------------------

// this is just dummy output to control program by itself
// output could be whatever
// after error retries device MUST be stoped and restarted
void ErrorLED(int cyc)
{
  for(int i=0;i<cyc;++i){
    digitalWrite(ERLED,HIGH);
    delay(500);
    digitalWrite(ERLED,LOW);
    delay(500);
  }
  while(1);
}

/*  Check if in message is char c
 *  UART is sending more than text in sms
 *  best to use for detecting one char
 *  needs timeout between sms's
 */
bool checkForChar(char c)
{
  char opChar = 'X';
  while(SmsSerial.available())
  {
    opChar = SmsSerial.read();
    if(opChar==c)
      return true;
  }
  return false;
}

bool sendToUART(const char* command, const char* answer)
{
  bool recieved = false;
  memset(readStr,0,BUFF_SIZE);
  index=0;  
  
  for(int i = 0 ; i<3; ++i)
  {
    SmsSerial.println(command); //send actuall command
    delay(500);
    /*
    //sends command from PC to SmsSerial
    while (Serial.available()) 
    {
      SmsSerial.write(Serial.read()); 
    }
    */
    readInput();
    //displays SmsSerial output strored in array
    Serial.write(readStr);
    //looks for answer from SmsSerial
    if(strstr(readStr,answer)!=NULL)
    {        
        recieved=true;
        break;
    }
    delay(700);
    
  }
  if(recieved==false)
  {
    return false;
  }
  return true;
}
/*readInput
 * reads input from SmsSerial
 * stores output to global array readStr
 * if message is too long, array will be erased without saving previous context
 */
void readInput(){
  while(SmsSerial.available())
  {
    readStr[index++] = SmsSerial.read();
    if(index==BUFF_SIZE)
    {
      memset(readStr,0,BUFF_SIZE);
      index=0;  
    }
  }  
}
