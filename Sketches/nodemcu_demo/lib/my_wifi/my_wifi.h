#ifndef my_wifi_h
#define my_wifi_h

#include <ESP8266WiFi.h>

//wifi states
#define WIFI_STATE_UNDEFINED 0
#define WIFI_STATE_NOT_SETUP 1
#define WIFI_STATE_SETTING_UP 2
#define WIFI_STATE_SETUP_FAILED 3
#define WIFI_STATE_OK 4
#define WIFI_STATE_ERROR 5

void SetupWifi( const char* ssid, const char* password );
void StartMQTT();

void DoHttpWiFi( WiFiServer& server, void (*Toggle)(), void (*Switch)( bool ) );
void DoMQTT();

byte GetWifiState();

String CreateHtmlStatus();
String CreateHtmlMode();
String CreateHtmlState();

void EspSend( String content, WiFiClient& client );

#endif
