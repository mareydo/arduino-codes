
#ifndef GPRSMODULE_H
#define GPRSMODULE_H
/// @cond INCLUDES

void initPins();
bool handshake();
bool initSettings();
void dumpInitData();
void delayCustom (const unsigned int);

void flushSerials();
bool recieveAT(const char*);
bool sendAT(const char*, const char*,const bool);

bool initSendSMS();
bool receiveSMSBody(char*, const unsigned char);
bool sendSMS(const char*);
bool receiveSMS(char*, const unsigned char);

bool showGps();
bool readGPS();
bool getPosition();

bool checkFormat(const char*);
bool parseInitSMS();
char parseControlSMS();
void sendCoordsSMS();
bool receivedPasswordSMS();
/// @endcond
#endif
