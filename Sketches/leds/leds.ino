int ALedPin = 13;                 // LED connected to digital pin 13
int BLedPin = 12;                 // LED connected to digital pin 12

int relayPin = 2;
boolean relayOn = true;

void setup() {
  pinMode(ALedPin,OUTPUT);
  pinMode(BLedPin,OUTPUT);
  //pinMode(relayPin,OUTPUT);
}

void loop() {
  if( relayOn )
  {
    relayOn = false;
    digitalWrite(ALedPin, LOW);    // sets the LED off
    digitalWrite(BLedPin, HIGH);    // sets the LED on
    //digitalWrite(relayPin, LOW);
  }
  else
  {
    relayOn = true;
    digitalWrite(ALedPin, HIGH);   // sets the LED on
    digitalWrite(BLedPin, LOW);   // sets the LED off
    //digitalWrite(relayPin, HIGH);
  }
  
  
  delay( 2000 );  
}
