#ifndef ultrasound_h
#define ultrasound_h

#include <ESP8266WiFi.h>

#define ULTRAECHO 5
#define ULTRATRIG 4
#define MaxUltraDistance 4000 // anything larger than this is considered an error and discarded
#define MinUltraDistance 0 // anything less than this is considered an error and discarded

void InitUltra();
float IntSampleUltra( byte TriggerPin, byte EchoPin );
float SampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );
float DblSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );
float SnglSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin );

#endif
