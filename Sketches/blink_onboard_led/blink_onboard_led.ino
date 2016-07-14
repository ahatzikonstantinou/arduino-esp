int ledPin = 13;                 // LED connected to digital pin 13
int relayPin = 2;

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
}

void loop() {
  flash( 1, 50 );
  relayFlash( 50 );
  delay( 2000 );
  /*
  for( int i = 1 ; i <= 10 ; i++ )
  {
    flash( i, 50 );
    delay( 1000 );
  }
  */
}

void relayFlash( int onDuration )
{
  digitalWrite(relayPin, HIGH);
  delay(onDuration);
  digitalWrite(relayPin, LOW);
}

void flash( int times, int blinkDelay )
{
  for( int i = 0 ; i < times ; i++ )
  {
    digitalWrite(ledPin, HIGH);   // sets the LED on
    delay(blinkDelay);                  // waits for a second
    digitalWrite(ledPin, LOW);    // sets the LED off
    delay(blinkDelay);                  // waits for a second
  }
}
