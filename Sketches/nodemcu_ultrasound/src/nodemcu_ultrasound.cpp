/*
********************************************
14CORE ULTRASONIC DISTANCE SENSOR CODE TEST
********************************************
*/
#define TRIGGER 4
#define ECHO    5
#define MAX_DISTANCE 200

#include <ESP8266WiFi.h>

// #include <NewPing.h>
// NewPing sonar(TRIGGER, ECHO, MAX_DISTANCE);

// NodeMCU Pin D1 > TRIGGER | Pin D2 > ECHO

void setup() {

  Serial.begin (115200);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT_PULLUP);
  // pinMode(BUILTIN_LED, OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(TRIGGER), Plugin_013_interrupt, CHANGE);
}

void loop() {

  // delay(1000);
  // unsigned int uS = sonar.ping_cm();
  // Serial.print(uS);
  // Serial.println("cm");

  long duration, distance;
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;

  Serial.print("Centimeter:");
  Serial.println(distance);
  delay(1000);
}
