#ifndef seven_seg_h
#define seven_seg_h

#include <Arduino.h>

//7 segment display pin assignments
#define a7Seg 14
#define b7Seg 12
#define c7Seg 3
#define d7Seg 15
#define e7Seg 13
#define f7Seg 2
#define g7Seg 0
#define dp7Seg 1

//wifi blinking
#define BLINK_WIFI_NONE -1  //do not blink
#define BLINK_WIFI_DP 0
#define BLINK_WIFI_BARS_SYNC 1
#define BLINK_WIFI_BARS_ASYNC 2
#define BLINK_WIFI_BARS_DP_SYNC 3

#define WIFI_FAIL_BLINK_TIME_ON 500000 // blink on microsecs when blinking wifi leds
#define WIFI_FAIL_BLINK_TIME_OFF 200000 // blink off microsecs when blinking wifi leds

void InitLeds();
void SetLeds();
void BlinkWifiLed();
void BlinkWifiLedStart( byte blinkMode );
void BlinkWifiLedStop();
void TurnLedsOff();
void SetLedsPerson();
void SetLedsOneWave();
void SetLedsTwoWave();
void SetLedsWifi();
void SetLeds();

#endif
