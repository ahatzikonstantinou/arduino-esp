#ifndef detect_h
#define  detect_h

#include <Arduino.h>

#define DetectStatusNotReadyYet 0
#define DetectPersonStatusIsPresent 1
#define DetectPersonStatusIsAbsent 2
#define OneWaveDetected 1
#define OneWaveNotDetected 2
#define TwoWaveDetected 1
#define TwoWaveNotDetected 2

byte DetectPersonPresent();
byte DetectPersonAbsent();
byte DetectOneWave();
byte DetectTwoWave();

unsigned int GetUltraDistPerson();  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
void SetUltraDistPerson( unsigned int d );
unsigned int GetUltraDistWave();  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
void SetUltraDistWave( unsigned int d );
unsigned long GetPersonPresentDelay(); // recheck after x millisenonds to ensure a person is still in front of the ultrasound sensor
void SetPersonPresentDelay( unsigned long d );
unsigned long GetPersonAbsentDelay(); // recheck after x millisenonds to ensure a person is still away from the ultrasound sensor
void SetPersonAbsentDelay( unsigned long d );
unsigned long GetTwoWaveDuration();  // the max duration of a two wave gesture
void SetTwoWaveDuration( unsigned long d );
unsigned long GetTwoWaveDelay();  // the delay betweeb two waves in a two wave gesture
void SetTwoWaveDelay( unsigned long d );
unsigned long GetOneWayDelay();  // the delay until the user moves the hand away for a one wave gesture
void SetOneWayDelay( unsigned long d );

#endif
