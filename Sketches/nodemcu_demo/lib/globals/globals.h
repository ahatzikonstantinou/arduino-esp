#ifndef globals_h
#define globals_h

#include <Arduino.h>

#define DEVICE_NAME_SIZE 30
char* GetDeviceName();


#define MODE_PERSON 0  //0: switch on when detect person
#define MODE_TWO_WAVE 1    //1: switch on when detect two gesture wave
#define MODE_ONE_WAVE 2    //2: switch on when detect single gesture wave
#define MODE_WIFI 3    //3: controlled by wifi only

const byte maxMode = MODE_WIFI;
byte GetMode();

bool GetRelayOn();
#define relayPin BUILTIN_LED

#endif
