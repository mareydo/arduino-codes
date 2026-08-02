
#define PIN_ECHO_1  2
#define PIN_TRG_1   3

#define PIN_ECHO_2  4
#define PIN_TRG_2   5

void generateTrigger(byte pin)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
}

void meassure(byte trgPin, byte echoPin)
{
 // generate 10-microsecond pulse to TRIG pin
  generateTrigger(trgPin);

  // measure duration of pulse from ECHO pin
  double duration_us = pulseIn(echoPin, HIGH);

  // calculate the distance
  double distance_cm = 0.017 * duration_us;

  // print the value to Serial Monitor
  Serial.print("TRG: ");
  Serial.println(trgPin);
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
}

void setup() {
  pinMode(PIN_TRG_1, OUTPUT);
  pinMode(PIN_TRG_2, OUTPUT);

  pinMode(PIN_ECHO_1, INPUT);
  pinMode(PIN_ECHO_2, INPUT);

  Serial.begin(9600);
}

void loop() {
  meassure(PIN_TRG_1, PIN_ECHO_1);

  delay(500);
}
