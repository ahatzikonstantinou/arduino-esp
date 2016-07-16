#include <ESP8266WiFi.h>

#define a7Seg 14
#define b7Seg 12
#define c7Seg 3
#define d7Seg 15
#define e7Seg 13
#define f7Seg 2
#define g7Seg 0
#define dp7Seg 1

void setup() {
  // put your setup code here, to run once:
  pinMode( a7Seg, OUTPUT );
  pinMode( b7Seg, OUTPUT );
  pinMode( c7Seg, OUTPUT );
  pinMode( d7Seg, OUTPUT );
  pinMode( e7Seg, OUTPUT );
  pinMode( f7Seg, OUTPUT );
  pinMode( g7Seg, OUTPUT );
  pinMode( dp7Seg, OUTPUT );

  digitalWrite( a7Seg, HIGH );
  digitalWrite( b7Seg, HIGH );
  digitalWrite( c7Seg, HIGH );
  digitalWrite( d7Seg, HIGH );
  digitalWrite( e7Seg, HIGH );
  digitalWrite( f7Seg, HIGH );
  digitalWrite( g7Seg, HIGH );
  digitalWrite(dp7Seg, HIGH );
  delay( 3000 );
  // pinMode( BUILTIN_LED,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // digitalWrite(BUILTIN_LED,LOW);
  // delay(500);
  // digitalWrite(BUILTIN_LED,HIGH);
  // delay(1000);
  digitalWrite( a7Seg, LOW );
  delay( 500 );
  digitalWrite( a7Seg, HIGH );

  digitalWrite( b7Seg, LOW );
  delay( 500 );
  digitalWrite( b7Seg, HIGH );

  digitalWrite( c7Seg, LOW );
  delay( 500 );
  digitalWrite( c7Seg, HIGH );

  digitalWrite( d7Seg, LOW );
  delay( 500 );
  digitalWrite( d7Seg, HIGH );

  digitalWrite( e7Seg, LOW );
  delay( 500 );
  digitalWrite( e7Seg, HIGH );

  digitalWrite( f7Seg, LOW );
  delay( 500 );
  digitalWrite( f7Seg, HIGH );

  digitalWrite( g7Seg, LOW );
  delay( 500 );
  digitalWrite( g7Seg, HIGH );

  digitalWrite( dp7Seg, LOW );
  delay( 500 );
  digitalWrite( dp7Seg, HIGH );

}
