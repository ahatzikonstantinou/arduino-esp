#ifndef utils_h
#define utils_h

#include <Arduino.h>

#define LOG_ACTOR_WIFI 0
#define LOG_ACTOR_BUTTON 1
#define LOG_ACTOR_ULTRASOUND 2
#define LOG_ACTOR_SYSTEM 3
#define LOG_TARGET_RELAY 0
#define LOG_TARGET_MODE 1
#define LOG_TARGET_NAME 2
#define LOG_TARGET_SYSTEM 3

void GoToNextMode();
void ChangeMode( int newMode );
void UpdateDeviceName( String name );

void Log( byte actor, byte target, String value );

void Toggle();
void Switch( bool on );

#endif
