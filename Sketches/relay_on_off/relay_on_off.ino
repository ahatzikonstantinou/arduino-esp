int ledPin = 13;                 // LED connected to digital pin 13
int relayPin = 2;
boolean relayOn = true;

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
}

void loop() {
  if( relayOn )
  {
    relayOn = false;
    digitalWrite(ledPin, LOW);    // sets the LED off
    digitalWrite(relayPin, LOW);
  }
  else
  {
    relayOn = true;
    digitalWrite(ledPin, HIGH);   // sets the LED on
    digitalWrite(relayPin, HIGH);
  }
  
  
  delay( 2000 );  
}
