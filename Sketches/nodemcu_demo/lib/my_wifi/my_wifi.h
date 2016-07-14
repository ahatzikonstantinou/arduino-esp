#ifndef my_wifi_h
#define my_wifi_h

#include <ESP8266WiFi.h>


void SetupWifi( const char* ssid, const char* password );

void DoWiFi( WiFiServer& server, void (*Toggle)(), void (*Switch)( bool ) );

#endif
