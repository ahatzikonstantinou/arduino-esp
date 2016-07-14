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


#define TWO_WAVE_DURATION 500UL  // the max duration of a two wave gesture
#define TWO_WAVE_DELAY 300UL  // the delay betweeb two waves in a two wave gesture
#define ONE_WAVE_DELAY 300UL  // the delay until the user moves the hand away for a one wave gesture

byte DetectPersonPresent();
byte DetectPersonAbsent();
byte DetectOneWave();
byte DetectTwoWave();
#endif
