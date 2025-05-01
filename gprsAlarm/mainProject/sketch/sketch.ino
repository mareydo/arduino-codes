/// @cond INCLUDES

#include "GPRSModule.h" // module functions

/// @endcond

/** @defgroup sketchMain Main Sketch File
 *  @{
 */
 
/**@defgroup states The automata states
 * @{ * 
 * @brief Main program loop is divided into several states of automata
 * @see  project folder/doc/states.png
 */

/** 
 * Internal states of finite automata
 */
enum STATE{ SETUP_FETCH,      /**< Wait for initSMS with mobile number and password */  
            SIGNAL_FETCH,     /**< Wait for SMS or shake sensor input */  
            GET_GPS,          /**< Read GPS data from module and sending it in SMS (valid or last valid)*/  
            ALARM_TRIGGERED,  /**< Send SMS that alarm is triggered and gps coords*/  
            ALARM_ON          /**< Turn relay(alarm) on and wait for SMS with password */  
           };

STATE ACT_STATE, NEXT_STATE;
/** @} */ //end of states group

/**
 * \defgroup pinout I/O pins
 * @brief Assignment of pin numbers to variables
 * @{
 */
byte RELAY = 6;               /*!< Relay - alarm/siren/powercutout */
byte ALARM_SENSOR = 3;        /*!< Sensor - shake sensor/movement sensor */
/** @} */ //end of pinout group

/**
 * \defgroup functions Main functions
 * @{ 
 * @brief All needed functions which run primarly on arduino (no main interaction on module HW)
 */
 
/**
 * @brief Blink builtin LED 10 times
 * 
 * Used as mark between correct init (setup) and main program (loop)
 *
 * @param void
 *
 * @retval void
 */
void initDoneBlink()
{
 for(int i = 0; i < 10; ++i)
 {
  digitalWrite(LED_BUILTIN, HIGH);
  
  delayCustom(1000);
  
  digitalWrite(LED_BUILTIN, LOW);
  
  delayCustom(500);
 }
}

bool triggered = false; /*!< Interrupt flag, true if alarm is on */

/**
 * @brief Interrupt handler
 * 
 * Sets flag triggered to true
 *
 * @param void
 *
 * @retval void
 */
void triggerAlarm()
{
  triggered = true;
}


/** 
 *  @brief Main setup of module interfaces and objects   
 *  
 */
void setup() 
{
//init module
  initPins();
//builtin led diode
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
//vibration sensor
  pinMode(ALARM_SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(ALARM_SENSOR), triggerAlarm, FALLING);
//relay
  pinMode(RELAY, OUTPUT);
//handshake by "AT" command
//try to send and recieve empty command
//!@see handshake
 if(!handshake()){                  /*! without handshake module cant be restarter SW way, FATAL ERROR */
  digitalWrite(LED_BUILTIN, HIGH);  /*! signals error by turning led on */
  while(1)
  ;
 } 
//init sequence of setup commands
//!@see initSettings
 if(!initSettings()){               /*! if problems dont resolve after SW restart, FATAL ERROR */
  digitalWrite(LED_BUILTIN, HIGH);  /*! signals error by turning led on  */
  while(1)
  ;
 } 
//init is done, blink led 10 times
 initDoneBlink();                 /*! at the end calls initDoneBlink() */
}

/**
 * @brief Main loop
 * 
 * Finite state automata
 * @see group states
*/

void loop() {  
  switch(ACT_STATE){
    case SETUP_FETCH:
                      {                       
                        if(parseInitSMS()){               // get number and password
                          triggered = false;
                          NEXT_STATE = SIGNAL_FETCH;
                        }
                        break;
                      }        
    case SIGNAL_FETCH:
                      {
                        digitalWrite(RELAY, LOW);
                        static bool firstSignalFetch = true; //to send menuSMS only once
                        /*
                         * Send SMS with instructions
                         * Is send everytime when state is entered from another state
                         */
                        if(firstSignalFetch){
                          sendSMS("SMS ALARM, send: #GPS to get position | #ALARM to trigger alarm");     // sending menu 
                          firstSignalFetch = false;
                        }
                        char cntrlSMS = parseControlSMS();                                          //etching controlling sms 
                        /*
                         * H <= show GPS coords
                         * A <= trigger alarm
                         * shaken <= sensor was activated
                         */
                        if(cntrlSMS =='H'){
                           NEXT_STATE = GET_GPS;
                           firstSignalFetch = true;
                        }
                        else if((triggered)||(cntrlSMS =='A')){
                          NEXT_STATE = ALARM_TRIGGERED;
                          firstSignalFetch = true;
                        }
                        break;
                      }
          
    case GET_GPS:     {
                        sendCoordsSMS();                                                            // send GPS coords and continue to next state
                        NEXT_STATE = SIGNAL_FETCH;
                        break;
                      }
    case ALARM_TRIGGERED:
                      {
                        sendSMS("ALARM TRIGGERED, after GPS SMS, send password to deactivate");     // send Info SMS
                        sendCoordsSMS();                                                            // send GPS coords and continue to next state
                        NEXT_STATE = ALARM_ON;
                        break;
                      }
    case ALARM_ON:
                      {
                        digitalWrite(RELAY, HIGH);                                                // turn alarm ON 
                        if(receivedPasswordSMS()){                                                // fetch password
                          NEXT_STATE = SIGNAL_FETCH;
                          triggered = false;                                                      // disable alarm variable
                        }
                        break;
                      }
    }
    ACT_STATE = NEXT_STATE;
  
}

/** @} */ // end of functions group
/** @} */ // end of Main Sketch File group
