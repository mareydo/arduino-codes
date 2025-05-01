/// @cond INCLUDES

#include <AltSoftSerial.h>  /**< Serial on non factory pins */
#include <TinyGPS++.h>      /**< Read GPS more easyly */
#include "GPRSModule.h"     /**< Declarations */
#include <Arduino.h>        /**< Basic Ardu commands */


/// @endcond
/** @defgroup sourceFile Source File
 *  @{
 */
/*******************************************************************************************************************************/  
/** @defgroup globalVariables Global variables
 *  @brief Initialization of global variables and global objects
 *  @{ 
 */
 
#define MAX_BUFFER 60         /*!< AltSoftSerial uses same sized buffer (for one cycle), valid messages won't exceed this limit*/

#define NUM_LEN 13            /*!< Length of number in format +123 123 123 123*/
#define PSWD_LEN 4            /*!< Password length, could be changed*/
#define POS_LEN 22            /*!< Buffer length to store GPS coords*/
// Arrays with added +1 for terminating '0'
char number[NUM_LEN+1] = {0};     /*!< Mobile number to communicate with */
char password[PSWD_LEN+1] = {0};  /*!< Password for alarm */
char lastPos[POS_LEN+1] = {0};    /*!< Last stored GPS position */
/*******************************************************************************************************************************/  
 
/*******************************************************************************************************************************/  
byte PWR = 7;                                       /*!< Pin for signal to turn gprs module on */
/*******************************************************************************************************************************/  
AltSoftSerial moduleSerial;                         /*!< Serial comunication with gprs*/
/*******************************************************************************************************************************/  
TinyGPSPlus moduleGPS;                            /*!< TinyGps++ object */
TinyGPSCustom latLetter(moduleGPS, "GPGGA", 3);   /*!< 3rd position in NMEA format N/S */
TinyGPSCustom lngLetter(moduleGPS, "GPGGA", 5);   /*!< 5th position in NMEA format W/E */
/*******************************************************************************************************************************/  

/** @} */ // end of group globalVariables

/** @defgroup delay Custom Delay Function
 *  
 *  @brief Because interrupt is used, delay() is not working
 *  @{ 
 */

/**
 * @brief Waiting for defined time
 * 
 * Looping for given ms
 * 
 * @param ms (time to wait in ms)
 *
 * @retval void
 */
 
void delayCustom (const unsigned int ms)
{
  unsigned long long stamp = millis();
  while( (millis()-stamp) <= ms )
  ;
}

/** @} */ // end of group delay

/** @defgroup atFunctions Functions handling AT commands
 *  
 *  @brief Sending/Receiving AT commands, flushing RX Serial buffers
 *  @{ 
 */

/**
 * @brief Clearing RX buffers
 * 
 * reading in loop all available characters
 * 
 * delay is used to not outrun incoming comunication  
 * 
 * @param void
 *
 * @retval void
 */
void flushSerials()
{
  while (1)
  {
    delayCustom (20);  
    if (Serial.available ())                                      //Serial to PC
    {
      while (Serial.available ()) Serial.read ();
    }
    else
    {
      break;
    }
  }
  while (1)
  {
    delayCustom (20);  
    if (moduleSerial.available ())                             //Serial to Module
    {
      while (moduleSerial.available ()) moduleSerial.read ();
    }
    else
    {
      break;
    }
  }
  delayCustom(500);                                                   
}

/**
 * @brief Recieving AT command
 * 
 * check if command includes response
 * 
 * maximal read length is MAX_BUFFER -1
 * 
 * function end reading Serial if string is found or buffer is filled
 * 
 * won't flush reminding data on RX
 * 
 * @param[in] response  - substring to check in incomming string, can be NULL for not testing
 * 
 * @retval true if command include response
 * 
 * @retval false if no command is send or response is longer than MAX_BUFFER - 1
 */
bool receiveAT(const char* response)
{
  int index = 0;
  char returnAT[MAX_BUFFER] = {0};
  unsigned long long stamp = millis();
  while( (millis()-stamp) <= 500 )
  {
    while(moduleSerial.available())
    {
      returnAT[index++] = moduleSerial.read();      
      if((response == NULL)||(strstr(returnAT, response)!= NULL))   //explicit NULL, strstr can't compare NULL
      {
        Serial.println(F("=======RECEIVED======="));
        Serial.println(returnAT);
        return true;
      }
  
      if(index == MAX_BUFFER-1)  //cut exceeding characters
      {             
        Serial.println(F("=======NOT RECEIVED MAX BUFFER======="));
        Serial.println(response);
        Serial.println(returnAT);        
        return false;    
      }
    }
  }
  
  return false;
}

/**
 * @brief sending AT commands
 * 
 * flushes serials before and after
 * 
 * command is tried 10 times with 3s window for response
 * 
 * @param[in] command   - command to send 
 * 
 * @param[in] response  - module must respond that phrase somewhere in output
 * 
 * @param[in] oneTry    - true to try send command only once
 * 
 * @retval true if command was send and properly answered
 * 
 * @retval false 10 incorrect sendings
 *  
 */

bool sendAT(const char* command, const char* response, const bool oneTry = false)
{
  Serial.println(F("=======================COMMAND SENDING========================")); //echo
  Serial.println(command); //echo
  
  for(int i=0; i <10 ; ++i)
  {                
    Serial.println(i);                      // number of tries
    flushSerials();                         // clearing RX to proper answer reading
    moduleSerial.println(command);          // command
    moduleSerial.write(26);                 // line break
     
    unsigned long long stamp = millis();    
    while( (millis()-stamp) <= 3000 )         /*! time for response 3s*/
    {                                         
      if(receiveAT(response))                // reading RX
      {  //received string matches response 
          Serial.println(F("=======================COMMAND RECEIVED======================="));         
          delayCustom(500);
          flushSerials();   // answer for command was delivered, everything after is not wanted
          return true;      // something was received and was answered correctly
      }     
    }
    delayCustom(700);             /*! 700ms delay between tries*/
    if(oneTry)      
    {
      break;
    }
  }
  Serial.println(F("=======================COMMAND NOT RECIEVED======================="));  
  flushSerials();         /*<Module didn't respond or didn't respond correctly, delete everything left */
  return false;           /*<timed out  */
}

/** @} */ // end of group atFunctions
/*******************************************************************************************************************************/  
/******************************************************************************************************************************* 
 *   INITIALIZATION                       
 *   DUMP INIT STRINGS                    
 *   INIT PINS AND SERIALS                
 *   AT -> OK <- HANDSHAKE                
 *   MODULE MODES SETTINGS                
*******************************************************************************************************************************/ 
/** @defgroup initFunctions Functions to init hardware
 *  @brief Setup pins ,serial comunication and module mods
 * @{ 
 */

/**
 * @brief Deleting initial data 
 *
 * reading and not storing inital comunication
 * 
 * fetch data for 7 seconds wirh 20ms loops
 * 
 * used as mark between correct init (setup) and main program (loop)
 *
 * @param void
 *
 * @retval void
 */
void dumpInitData()
{
  bool wasAvailable = false;
  unsigned long long stamp = millis();
  while( (millis()-stamp) <= 7000 )
  {    //delete everything for 7s
    while(moduleSerial.available())
    {
      moduleSerial.read();
      delayCustom(20);
      wasAvailable = true;
    }
    if(wasAvailable)
    {
      break;
    }
  }
}

 
/**
 * @brief Basic Pin and Object initialisation
 *
 * setup Serial with PC, Serial with module on 9600b
 * 
 * dumps inital RX
 * 
 * @see dumpInitData
 * 
 * @param void
 *
 * @retval void
 */
void initPins()
{
//PC SERIAL  
   Serial.begin(9600);                    
   Serial.println(F("[INIT]: BEGIN"));
//POWER BUTTON
   pinMode(PWR,OUTPUT);
   digitalWrite(PWR,HIGH);
   delayCustom(2000);           /*! hold pin in high for 2s*/
   digitalWrite(PWR,LOW);
//MODULE SERIAL
   moduleSerial.begin(9600);
//dump init uart RX in whatever form
   Serial.println(F("[INIT]: Dump"));
   dumpInitData();
   Serial.println(F("[INIT]: END"));
}

/**
 * @brief Do handshake with module
 *
 * tries for 5s to Send "AT" and recieve "OK"
 *
 * @param void
 *
 * @retval bool (true - if module responds "OK")
 */
bool handshake()
{
 Serial.println(F("[HANDSHAKE]: BEGIN"));
 
 unsigned long long stamp = millis();
 while( (millis()-stamp) <= 5000 )
 {       //try for 5s  
    if(sendAT("AT","OK"))
    {
      Serial.println(F("[HANDSHAKE]: DONE"));
      return true;
    }
 }
 Serial.println(F("[HANDSHAKE]: FAILED"));
 return false;
}

/**
 * @brief Initial setup of module
 *
 * uses AT commands
 * 
 * setup mode, GPS, comunication handling
 * 
 * all of them needs to be set up
 * 
 * tries to send commands 3 times, redundant setup doesn't matter
 * 
 * sends RST command after each fail and waits 3seconds
 * 
 * dump initial comunication after reboot
 * 
 * @see dumpInitData
 *
 * @param void
 *
 * @retval bool (true - if all commands had proper response)
 */
bool initSettings()
{
  for(int i = 0; i < 3; ++i)
  {
    if( sendAT("AT+CMGF=1","OK")  &&        /*! AT+CMGF=1" set to text mode*/
        sendAT("AT+GPS=1","OK")   &&        /*! AT+GPS=1 turn GPS on*/
        sendAT("AT+CNMI=2,2,0,0,0","OK") )  /*! AT+CNMI=2,2,0,0,0 forward to device*/
    {
      return true;
    }
    sendAT("AT+RST","OK");                  /*! after each failed try restart*/
    delayCustom(3000);
    dumpInitData();    
  }
  return false;                             // fatal error
}

/** @} */ //end of group initFunctions
/*******************************************************************************************************************************/ 
/******************************************************************************************************************************* 
 *  INIT MODULE FOR SENDING   
 *  SENDING SMS                                                 
/*******************************************************************************************************************************/ 
 
/**
 * \defgroup sendSMS All steps to send SMS
 * @brief Functions to setup module and send SMS
 * @{ 
 */
  
/**
 * @brief Initial setup for sending SMS
 *
 * sends "AT+CMGS="number" command
 * 
 * "number" - recipient mobile number in quotes
 * 
 * module will switch to SMS body text mode indicated with a '>'
 *
 * @param void
 *
 * @retval bool (true if module responds '>')
 */
bool initSendSMS()
{
  char setupString[24] = "AT+CMGS=\"";  // 9chars + 13chars for number format + 2
  strncat(setupString,number,NUM_LEN);
  strcat(setupString,"\0");
  return sendAT(setupString,">");       /*! whatever will be send after '>' response, is a body of the SMS*/
}

/**
 * @brief Handle SMS sending
 *
 * sends "AT+CMGS="number" command 
 * 
 * @see initSendSMS
 * 
 * (DEBUG BUG) response may not be accepted for the first time, because OK is after the message body, and input buffer will overflow with error
 *             check for Module return value manually in Serial monitor to see if correct text was recieved, if any errors occure
 *        
 * delay(3000) used as buffer time to call function multiple times in row
 * 
 * @param[in] message - message body to send
 *
 * @retval bool (true if module is set up and responded with "OK")
 */
bool sendSMS(const char* message)
{
  Serial.print(F("[SMS SEND]: Sending: "));
  Serial.println(message);
  if(!initSendSMS())
  {
    Serial.println(F("[SMS SEND]: Init failed"));
    return false;
  }
  
   char responseCheck[6]= {0};        //copy first 5chars of message to check against module echo, not reliable, change for OK if any bug occures
   strncpy(responseCheck,message,5);

  if(sendAT(message, responseCheck, true)) 
  {
    Serial.println(F("[SMS SEND]: Send OK"));
    delayCustom(3000);
    return true; //send body
  }
  delayCustom(3000);
  return false;
}

/** @} */ // end of group sendSMS
/*******************************************************************************************************************************/ 
/******************************************************************************************************************************* 
 *  RECEIVE SMS BODY
 *  HANDLE SMS RECEIVING
*******************************************************************************************************************************/ 

/**
 * \defgroup receiveSMS All steps to read SMS
 * @brief Listening on RX and parsing input, to check if SMS came
 * @{ 
 */
/**
 * @brief Read SMS and ignore control characters
 *
 * reads directly from RX
 * 
 * 1sec loop is used to not outrun serial reading
 *
 * @param[out] messageBody - buffer for storing message text 
 * 
 * @param[in] bodyLimit   - size of output buffer
 *
 * @retval bool (false if there are still data and: buffer is full or after 1s timeot)
 */
bool receiveSMSBody(char* messageBody, const unsigned char bodyLimit)
{ 
  unsigned char index = 0;
  unsigned long long stamp = millis();
  while( (millis()-stamp) <= 1000 )
  {   
    while(moduleSerial.available())
    {
      messageBody[index] = moduleSerial.read();
      if( (!isControl(messageBody[index]))||(messageBody[index]==' ') ) /*! jump over not printable text characters or not space*/
      {
        index++;
      }
      if( index == bodyLimit )
      {
        Serial.println(F("[SMS RECIEVE]: BODY LIMIT"));
        return (!moduleSerial.available());
      }
    }
  }
  
  return (!moduleSerial.available());
}

/**
 * @brief Handle recieving SMS
 *
 * listens on RX for 10s
 * 
 * check for '+CMT: "number'
 * 
 * "number" is global variable, can be empty, then only '+CMT: " ' is checked
 * 
 * after recieving number will jump over 3x ' " ' to get to message body 
 * 
 * (+CMT: "number",,"time")
 * 
 * flushes serials after returning from receiveSMSBody function
 * 
 * @see receiveSMSBody
 * 
 * @param[out] message    - buffer for storing message text 
 * 
 * @param[in] messageLength   - size of output buffer
 *
 * @retval true if receiveSMSBody returns true
 * @retval false after 10s timeout
 */ 
bool receiveSMS(char* message, const unsigned char messageLength)
{
  char checkAT[7+NUM_LEN+1] = {0}; //7chars + number
  bool correctNumber = false;
  strcpy(checkAT,"+CMT: \"");
  if(strlen(number)!=0)
  {
    strncat(checkAT,number, NUM_LEN);       // CMT: "number
  }
  unsigned long long stamp = millis();
  Serial.println("[SMS BODY]");
  while( (millis()-stamp) <= 10000 )
  {   
    if( (receiveAT(checkAT)) && (!correctNumber) )    // receive something and check only once
    {
      Serial.println(F("====[SMS BODY]: NUMBER RECEIVED===="));
      while((moduleSerial.available())&&(moduleSerial.read()!='\"')) delayCustom(20);
      while((moduleSerial.available())&&(moduleSerial.read()!='\"')) delayCustom(20);
      while((moduleSerial.available())&&(moduleSerial.read()!='\"')) delayCustom(20);
      correctNumber = true;                           // got number, don't control again
    }
    if(correctNumber)
    {
      if( receiveSMSBody(message, messageLength) )
      {
        Serial.println(F("[SMS RECEIVE]: BODY:"));
        Serial.println(message);
        Serial.println(F("[SMS RECEIVE]: DONE"));
        flushSerials();
        return true;
      }
    }
  }
  flushSerials();
  return false;  //nothing received or wrong format of message
}

/** @} */ // end of group receiveSMS
/*******************************************************************************************************************************/ 
/******************************************************************************************************************************* 
 *   SHOW AND SAVE GPS LOCATION           
 *   READ LOCATION WITH TINYGPS++         
 *   GET AND SAVE POSITION API            
*******************************************************************************************************************************/ 
 
/**
 * \defgroup GPSHandler Functions to handle GPS comunication
 * @brief Reading location using TinyGPS++ module
 * @{ 
 */
 
/**
 * @brief Store GPS coords
 *
 * Read location in tinyGps++ object moduleGps
 * 
 * won't turn GPS on, only checks already loaded value
 * 
 * stores into global variable lastPos if valid
 * 
 * before storing new value wipes lastPos with 0
 * 
 * N/S LAT W/E LONG
 *
 * @param void
 * 
 * @retval bool (true if location was valid by tinyGPS++)
 */ 
bool showGps()
{
  if(moduleGPS.location.isValid())            // got some satelites and read valid location
  {
    Serial.println(F("[GPS]: VALID LOCATION"));
    memset(lastPos, 23, 0);  
    char longitude[10] = {0};
    dtostrf(moduleGPS.location.lat(),9,6,lastPos+1);    // chars including '.' with 6 decimals -90 to +90
    dtostrf(moduleGPS.location.lng(),10,6,lastPos+12);  // 10 chars with 6 decimals -180 to + 180
 // save to lastPos   
    lastPos[10]= ' ';
    strncpy(lastPos,latLetter.value(),1);
    strncpy(lastPos+11,lngLetter.value(),1);
    
    Serial.print(F("LAT=")); Serial.print(latLetter.value()); Serial.println(moduleGPS.location.lat(), 6);    
    Serial.print(F("LONG=")); Serial.print(lngLetter.value()); Serial.println(moduleGPS.location.lng(), 6);  
    Serial.println(lastPos);
    return true;
  }
  return false;
}
  

/**
 * @brief Reading GPS info on serial
 * 
 * listens for 10s
 * 
 * then validate data
 * 
 * @see showGps
 * 
 * will flush remaining data on Serial
 * 
 * @retval bool (true if recieved location is valid)
 */
bool readGPS()
{
  unsigned long stamp = millis();
  while(millis() - stamp < 10000) //10s timeout for tinyGps++
  {        
    while(moduleSerial.available())
    {     
      if(moduleGPS.encode(moduleSerial.read())) // new word encoded
      {
        if(showGps()) {                   ///check if word makes position valid, if yes break
          flushSerials();
          return true;
        }
      }
      
    }
  }
  flushSerials();
  return false;                           // timed out
}

/**
 * @brief Handle GPS comunication
 *
 * turn GPS on with AT+GPSRD=1
 * 
 * after readGPS call turn GPS off with AT+GPSRD=1
 * 
 * @see readGPS
 *  
 * @param void
 *
 * @retval bool (true if all commands executed right and lastPos was updated)
 */
bool getPosition()
{
  Serial.println(F("[GPS]:GET"));
  bool validRead = false;
//!turn GPS on 
  if(!(sendAT("AT+GPSRD=1","OK"))) return false;
//!read data
  if((readGPS())) validRead=true;
//!turn GPS off
  if(!(sendAT("AT+GPSRD=0","OK"))) return false;
  Serial.print(F("[GPS]return: ")); Serial.println(validRead);
//!flush Serial
  flushSerials();
//!return
  return validRead;
}

/** @} */ // end of group GPSHandler
/*******************************************************************************************************************************/  
/**
 * \defgroup Parsers Function to handle string parsing
 * @brief Handling controlling SMS bodies 
 * @{ 
 */

/**
 * @brief Check initSMS format
 *
 * checking input string:
 * 
 * wanted format +123123123123 - 1234
 * 
 * wont check after right format => +123123123123 - 123456789 is correct with password 1234
 * 
 * @param[in] str - string to check format in
 *
 * @retval bool (true if input string is in right format)
 */
bool checkFormat(const char* str)
{
  Serial.print(F("[SMS INIT]:Check format"));
  Serial.println(str);
  const char* ptr = str;
//+
  if((ptr==NULL) ||(*ptr != '+')) 
  {
    Serial.println(F("[SMS INIT]:ERR-No plus"));
    return false;
  }
  ptr+=1;
//number
  for(int i = 0; i < 12; ++i, ++ptr)
  {
    if( (ptr==NULL) || (!isdigit(*ptr)) )
    {
      Serial.println(F("[SMS INIT]:ERR-Wrong number"));
      return false;
    }
  }
//' - '
  if(strncmp(ptr," - ",3)!= 0)
  {
    Serial.println(F("[SMS INIT]:ERR-No dash"));
    return false;
  }
  ptr+=3;
//password
  for(int i = 0; i < 4; ++i, ++ptr)
  {
    if( (ptr==NULL) || (!isdigit(*ptr)) )
    {
      Serial.println(F("[SMS INIT]:ERR-Wrong password"));
      return false;
    }
  }
  Serial.println(F("[SMS INIT]:Format OK"));
  return true;
}


/**
 * @brief Parse initialization SMS
 *
 * receive sms
 * 
 * @see receiveSMS
 * 
 * check format of body
 * 
 * @see checkFormat
 * 
 * store in global variables number and password
 * 
 * wanted format "number - password"
 * 
 * will accept SMS from any number
 * 
 * @param void
 *
 * @retval bool (true if correct SMS Body is recieved from anyone)
 */
bool parseInitSMS()
{
  char messageBody[NUM_LEN+3+PSWD_LEN+1] = {0}; // 3 for ' - ' 1 for \0
  char* cpyPtr = messageBody;
  if(receiveSMS(messageBody, NUM_LEN+3+PSWD_LEN) && checkFormat(messageBody) ){
    strncpy(number, cpyPtr, NUM_LEN);
    cpyPtr+=16;
    strncpy(password, cpyPtr, 4);
    Serial.print(F("[SMS INIT]:Got correct data"));
    Serial.print(number);
    Serial.print(" & ");
    Serial.println(password);
    return true;
  }
  return false;
}

/**
 * @brief Parse received SMS as menu switch
 *
 * readingSMS with 7 char buffer
 * 
 * @see receiveSMS
 * 
 * check for "#ALARM" or "#GPS"
 * 
 * '#' to make SMS uniqe and not recieve false commands by mistake from user
 *
 * @param void
 *
 * @retval char 'X'/'H'/'A'
 * 
 *        X = nothing
 *        
 *        H = user wants GPS coords
 *        
 *        A = user wants forced alarm
 */
char parseControlSMS()
{
  char messageBody[7] = {0};
  if(receiveSMS(messageBody, 6))
  {
    if(strstr(messageBody,"#ALARM")){
      return 'A';  
    }
    if(strstr(messageBody,"#GPS")){
      return 'H';  
    }
  }
  return 'X';
}
/**
 * @brief Get position and send it in SMS
 *
 * readingGPS
 * @see getPosition
 * 
 * sends either new position or last known (stored in lastPos)
 * 
 * tries to send SMS 3 times
 * 
 * @param void
 *
 * @retval void
 */
void sendCoordsSMS()
{
  if(!getPosition())
  {
    char message[50] = "NO GPS, last known: ";
    strcpy(message+20,lastPos);
    strcat(message,'\0');
    sendSMS(message);
  }
  else
  {
    for(int i = 0; i < 3 ; ++i)
    {
      if(sendSMS(lastPos))
      {
        break;
      }
    }
  }
}
/**
 * @brief Read SMS to disable alarm
 *
 * reading SMS bodies and checking if they contain password
 * 
 * checking agains global variables password and number
 * 
 * must be used after initialization of global variables (for meaningfull reason)
 * 
 * @see receiveSMS
 * 
 * @see parseInitSMS
 * 
 * @param void
 *
 * @retval bool (true if password was in correctly received message)
 */
bool receivedPasswordSMS()
{
  char messageBody[5] = {0};
  if(!receiveSMS(messageBody, 4)) return false;
  return( strcmp(password,messageBody)==0 );
}


/** @} */ //end of group Parsers
/*******************************************************************************************************************************/  
/** @} */ //end of group Source File
