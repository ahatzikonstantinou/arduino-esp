// #include <Arduino.h>
#include <ESP8266WiFi.h>

#define ECHO 5
#define TRIG 4
#define LED BUILTIN

#define dbg Serial


void setup()
{
  dbg.begin( 115200 );
  pinMode( TRIG, OUTPUT );
  pinMode( ECHO, INPUT );
}

void loop()
{
  long duration, distance;
  digitalWrite(TRIG, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(TRIG, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  if (distance < 4)
  {  // This is where the LED On/Off happens
    digitalWrite(LED,HIGH);
  }
  else
  {
    digitalWrite(LED,LOW);
  }

  if (distance >= 200 || distance <= 0)
  {
    dbg.println("Out of range");
  }
  else
  {
    dbg.print(distance);
    dbg.println(" cm");
  }
  // delay(500);
}
