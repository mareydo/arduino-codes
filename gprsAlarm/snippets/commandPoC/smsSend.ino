#include <SoftwareSerial.h>
char phone_no[]="+421907637282";

SoftwareSerial mySerial(2, 3); // RX, TX

void setup()
{
  // Open serial communications to computer
  Serial.begin(115200);

  mySerial.begin(115200); // Default for the board
  while(!Serial);
  while(!mySerial); 
  //Clear out any waiting serial data
  while (mySerial.available())
  {
    mySerial.read();
  } 
  Serial.println("beg");
  Serial.flush();
  
  delay(500);
  
}

void loop()
{
  
  while(!Serial.available())
  
    mySerial.write(Serial.read());
  
  
  if (mySerial.available())
  {
    Serial.write(mySerial.read());
  }
}

void sendSMS(){
  Serial.println("AT+CMGF=1");    
delay(2000);
Serial.print("AT+CMGS=\"");
Serial.print(phone_no); 
Serial.write(0x22);
Serial.write(0x0D);  // hex equivalent of Carraige return    
Serial.write(0x0A);  // hex equivalent of newline
delay(2000);
Serial.print("GSM A7 test message!");
delay(500);
Serial.println (char(26));//the ASCII code of the ctrl+z is 26

  }
