#ifndef ultrasound_h
#define ultrasound_h

#include <ESP8266WiFi.h>

#define ULTRAECHO 5
#define ULTRATRIG 4
#define MaxUltraDistance 4000 // anything larger than this is considered an error and discarded
#define MinUltraDistance 0 // anything less than this is considered an error and discarded

#define ULTRA_DIST_PERSON 100  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
#define ULTRA_DIST_WAVE 30  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
#define PERSON_PRESENT_DELAY 2000UL // recheck after x millisenonds to ensure a person is still in front of the ultrasound sensor
#define PERSON_ABSENT_DELAY 4000UL // recheck after x millisenonds to ensure a person is still away from the ultrasound sensor

void InitUltra();
float IntSampleUltra( byte TriggerPin, byte EchoPin );
float SampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );
float DblSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );
float SnglSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );

#endif
