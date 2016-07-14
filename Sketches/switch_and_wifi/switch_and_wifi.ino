#include "EEPROM.h"

#define EEsize 1024 // size in bytes of arduino UNO EEPROM
#define RelayStateAddress 0  //where the relay state is saved
#define LEDStateAddress 1  //where the LED state is saved
#define NetworkDataAddress 2  // where the networkData is saved

// NOTE: You cannot use IR, BUTTON and OPTO at the same time. Arduino has only 2 external interrupts available
// i.e. interrupt 0 on pin 2, and interrupt 1 on pin 3. In the following setting the optocoupler is set to pin 3
// in order to have everything in one source code file, but only 2 sensors can be used with external interrupts
#define IR 3
#define BUTTON 2
#define LED 13
#define OPTO 3 // optocoupler goes on an external interrupt e.g. pin 3.

volatile int ledState = LOW;
#define debounceButtonDelay 50 // 100 msecs to guard against button debounce
volatile int lastButtonCallTime = 0;
volatile int lastButtonState = 0;
volatile int lastIRCallTime = 0;
#define debounceIRDelay 100 // 100 msecs to guard against IR debounce
volatile int lastOPTOCallTime = 0;
#define debounceOPTODelay 100 // 100 msecs to guard against IR debounce

// comment this part out if not using LCD debug

#define BUFFER_SIZE 512

#define WIFI_SSID  "Wind WiFi 172839"      // change this to match your WiFi SSID
#define PASS  "312ggp12"  // change this to match your WiFi password
#define PORT  "8080"           // using port 8080 by default
#define AP_PORT "1025"    // port used for tcp server when wifi is in Access Point (PROGRAMMING) mode
#define WIFI_WORKING 0      // in this mode the wifi is reading incoming packets
#define WIFI_PROGRAMMING 1  // in this mode the wifi get in Access Point mode to get connection data
#define WIFI_PROGRAMMING_DELAY 3000000  // in microsecs. Hold the button for that long to get wifi in and out of programming mode
volatile byte WifiMode = WIFI_WORKING;
volatile byte lastWifiMode = WIFI_WORKING;

#define AUTOHOME "AutoHome"
#define AUTOHOME_VERSION "1.0"

typedef struct
{
  char NetSSID[60];
  char Password[30];
  char Name[60];
  char AutoHomeServerIP[15];
  char AutoHomeServerPort[5];
  char LogServerIP[15];
  char LogServerPort[5];
  int    BroadcastIntervalSecs;
} NetworkConnectionData;
NetworkConnectionData networkData;

char buffer[BUFFER_SIZE];


#define relayPin 9

#include <TimerOne.h> // Header file for TimerOne library
#define UltraSamplePeriod 2000000 // microseconds
#define UltraSampleNum 10  //how many samples to take every UltraSamplePeriod
#define UltraSampleInterval 10000  // microseconds to wait between each ultra sample
int ultraSamples[UltraSampleNum];
#define ULTRAECHO 11
#define ULTRATRIG 12
#define ULTRA_DIST_THRESHHOLD 20  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.

// If using Software Serial for debug
// Use the definitions below
#include <SoftwareSerial.h>
SoftwareSerial esp(8,7);  // use pins 7, 8 for software serial 
#define dbg Serial

/*
// If your MCU has dual USARTs (e.g. ATmega644)
// Use the definitions below
#define dbg Serial    // use Serial for debug
#define esp Serial1   // use Serial1 to talk to esp8266
*/


void dumpNetworkData()
{
  NetworkConnectionData n = networkData;
  dbg.println( F("Network Connection Data:") );
  dbg.print( F("NetSSID = ") ); dbg.println( n.NetSSID );
  dbg.print( F("Password = ") ); dbg.println( n.Password );
  dbg.print( F("Name = ") ); dbg.println( n.Name );
  dbg.print( F("AutoHomeServerIP = ") ); dbg.println( n.AutoHomeServerIP );
  dbg.print( F("AutoHomeServerPort = ") ); dbg.println( n.AutoHomeServerPort );
  dbg.print( F("LogServerIP = ") ); dbg.println( n.LogServerIP );
  dbg.print( F("LogServerPort = ") ); dbg.println( n.LogServerPort );
  dbg.print( F("BroadcastIntervalSecs = ") ); dbg.println( n.BroadcastIntervalSecs );
}

void InitNetworkData( NetworkConnectionData data );

void setup() 
{
  InitNetworkData( networkData);
  // assume esp8266 operates at 115200 baud rate 57600 38400 19200 9600 4800
  // change if necessary to match your modules' baud rate
  esp.begin(9600);
  
  dbg.begin(9600);
//  dbg.println("begin.");
    

  // print device IP address
//  dbg.print("device ip addr:");
//  esp.println("AT+CIFSR");
//  wait_for_esp_response(1000);
  
  //setup relay
  SetupRelay();
  dbg.println("Relay setup finished");
  
  //setup button
  // our button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON, INPUT_PULLUP );
  attachInterrupt (0, ButtonChange, CHANGE);  // attach interrupt handler
  dbg.println("Button setup finished");
  
  // setup the IR proximity sensor, it goes to LOW when an obstacle is detected
  // commented out in order to test the optocoupler on pin 3
  pinMode( IR, INPUT_PULLUP );
  attachInterrupt (1, IRChange, CHANGE);
  
  //setup LED
  pinMode (LED, OUTPUT);  // so we can update the LED
  digitalWrite( LED, GetLedState() ); // set the LED according to what is saved in EEPROM
  
  // setup optocoupler
//  pinMode( OPTO, INPUT_PULLUP );
//  attachInterrupt (1, OPTOChange, CHANGE);

  // setup ultrasound sensor
//  pinMode( ULTRATRIG, OUTPUT );
//  pinMode( ULTRAECHO, INPUT );
  
  // setup timer  
  Timer1.initialize(3000000); //microsecs
  Timer1.attachInterrupt( ToggleWifiMode );
  Timer1.stop();    //stop the counter
  
  //read netowrk connection data from eeprom
  EEPROM.get( NetworkDataAddress, networkData );
  dbg.println( "At setup after reading from EEPROM networkData:" );
  dumpNetworkData();
}

void InitNetworkData( NetworkConnectionData data )
{
  for( int i=0 ; i<60 ; i++ ) data.NetSSID[i] = 0;
  for( int i=0 ; i<30 ; i++ ) data.Password[i] = 0;
  for( int i=0 ; i<60 ; i++ ) data.Name[i] = 0;
  for( int i=0 ; i<15 ; i++ ) data.AutoHomeServerIP[i] = 0;
  for( int i=0 ; i<5 ; i++ ) data.AutoHomeServerPort[i] = 0;
  for( int i=0 ; i<15 ; i++ ) data.LogServerIP[i] = 0;
  for( int i=0 ; i<5 ; i++ ) data.LogServerPort[i] = 0;
}

// By default we are looking for OK\r\n
char OKrn[] = "OK\r\n";
byte wait_for_esp_response(int timeout, char* term=OKrn) 
{
  unsigned long t=millis();
  bool found=false;
  int i=0;
  int len=strlen(term);
  // wait for at most timeout milliseconds
  // or if OK\r\n is found
  while(millis()<t+timeout) {
    if(esp.available()) {
      buffer[i++]=esp.read();
      if(i==BUFFER_SIZE)  i=0;  // ahat: guard against buffer overflow
      if(i>=len) {
        if(strncmp(buffer+i-len, term, len)==0) {
          found=true;
          break;
        }
      }
    }
  }
  buffer[i]=0;
  dbg.print(buffer);
  return found;
}

bool read_till_eol() 
{
  static int i=0;
  if(esp.available()) {
    buffer[i++]=esp.read();
    if(i==BUFFER_SIZE)  i=0;
    if(i>1 && buffer[i-2]==13 && buffer[i-1]==10) {
      buffer[i]=0;
      i=0;
      dbg.print(buffer);
      return true;
    }
  }
  return false;
}



boolean wifiIsSetAsClient = false;
boolean wifiIsSetAsAccessPoint = false;
void loop() 
{
//  dbg.println( "In loop" );
  int ch_id, packet_len;
  char *pb;  
  if( WifiMode == WIFI_WORKING )
  {
//    dbg.println( "In WIFI_WORKING" );
    if (!wifiIsSetAsClient) 
    {
      SetupWiFiAsClient();
      wifiIsSetAsClient = true;
      wifiIsSetAsAccessPoint = false;
    }

//    dbg.println( "Before read_till_eol" );
    if(read_till_eol()) 
    {
      dbg.print( F("Read wifi buffer: ") );
      dbg.println( buffer );
      if(strncmp(buffer, "+IPD,", 5)==0) 
      {
        // request: +IPD,ch,len:data
        sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);
        if (packet_len > 0) 
        {
          // read serial until packet_len character received
          // start from :
          pb = buffer+5;
          while(*pb!=':') pb++;
          pb++;
          if( strncmp(pb, "GET /open", 9 ) == 0) 
          {
            wait_for_esp_response(1000);
            dbg.println(F("received open."));
            SwitchRelay( true );
            dbg.println(F("-> serve homepage"));
            serve_homepage(ch_id, true, true);
          }
          else if( strncmp(pb, "GET /close", 9 ) == 0) 
          {
            wait_for_esp_response(1000);
            dbg.println(F("received close."));
            SwitchRelay( false );
            dbg.println(F("-> serve homepage"));
            serve_homepage(ch_id, true, false);
          }
          else if (strncmp(pb, "GET /", 5) == 0) 
          {
            wait_for_esp_response(1000);
            dbg.println(F("received:"));
            dbg.println(pb);
            dbg.println(F("-> serve homepage"));
            serve_homepage(ch_id, false, false);
          }
        }
      }
    }
//    dbg.println( "After read_till_eol" );
  }
  else if( WifiMode == WIFI_PROGRAMMING )
  {
    if( !wifiIsSetAsAccessPoint )
    {
      SetupWiFiAsAccessPoint();
      wifiIsSetAsAccessPoint = true;
      wifiIsSetAsClient = false;
    }

    if(read_till_eol()) 
    {
      
      dbg.print( F("Read wifi buffer: ") );
      dbg.println( buffer );

      if(strncmp(buffer, "+IPD,", 5)==0) 
      {
        // request: +IPD,ch,len:data
        sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);
        if (packet_len > 0) 
        {
          // read serial until packet_len character received
          // start from :
          pb = buffer+5;
          while(*pb!=':') pb++;
          pb++;
          dbg.println(F("received:"));
          dbg.println(pb);
          ProgrammingProtocol( ch_id, pb );
        }
      }
    }
  }
}

void ProgrammingProtocol( int ch_id, char* pb )
{
  if( strncmp(pb, AUTOHOME, 8 ) == 0) 
  {
    wait_for_esp_response(1000);
    String text = AUTOHOME;
    text += " ";
    text += AUTOHOME_VERSION;
    text += F("\r\n");
    ESPSend( ch_id, text );
  }
  else
  {
    char command[21];
    char value[128];
    for( int i = 0 ; i<21 ; i++ ) command[i] = 0;
    for( int i = 0 ; i<128 ; i++ ) value[i] = 0;
        
    sscanf( pb, "%21[^:]:%128[^\n]", &command, &value );
    dbg.print( ", value: " ); dbg.println( value );
    if( strncmp(pb, "SetSSID", 7 ) == 0)
    {
      strncpy( networkData.NetSSID, value, 60 );
      ESPSend( ch_id, "SetSSID:ok\n" );
    } 
    else if( strncmp(pb, "SetPassword", 11 ) == 0)
    {
      strncpy( networkData.Password, value, 30 );
      ESPSend( ch_id, F("SetPassword:ok\n") );
    } 
    else if( strncmp(pb, "SetName", 7 ) == 0)
    {
      strncpy( networkData.Name, value, 60 );
      ESPSend( ch_id, F("SetName:ok\n") );
    } 
    else if( strncmp(pb, "SetAutoHomeServerIP", 19 ) == 0)
    {
      strncpy( networkData.AutoHomeServerIP, value, 15 );
      ESPSend( ch_id, F("SetAutoHomeServerIP:ok\n") );
    } 
    else if( strncmp(pb, "SetAutoHomeServerPort", 21 ) == 0)
    {
      strncpy( networkData.AutoHomeServerPort, value, 5 );
      ESPSend( ch_id, F("SetAutoHomeServerPort:ok\n") );
    } 
    else if( strncmp(pb, "SetLogServerIP", 14 ) == 0)
    {
      strncpy( networkData.LogServerIP, value, 15 );
      ESPSend( ch_id, "SetLogServerIP:ok\n" );
    } 
    else if( strncmp(pb, "SetLogServerPort", 16 ) == 0)
    {
      strncpy( networkData.LogServerPort, value, 5 );
      ESPSend( ch_id, "SetLogServerPort:ok\n" );
    } 
    else if( strncmp(pb, "SetInterval", 11 ) == 0)
    {
      networkData.BroadcastIntervalSecs = atoi( value );
      ESPSend( ch_id, "SetInterval:ok\n" );
      EEPROM.put( NetworkDataAddress, networkData );
    }
    else if( strncmp(pb, "StartWorkingMode", 16 ) == 0)
    {
      ESPSend( ch_id, "StartWorkingMode:ok\n" );
      WifiMode = WIFI_WORKING;
      dbg.println( F("Received StartWorkingMode from Wifi. WifiMode set to WIFI_WORKING") );
    }    
    else
    {
      String msg = command;
      msg += F(":ERROR");
      ESPSend( ch_id, msg );
    }
  }
}

void ESPSend( int ch_id, String text )
{
  esp.print( F("AT+CIPSEND=") );
  esp.print(ch_id);
  esp.print(",");
  esp.println(text.length());
  if(wait_for_esp_response(2000, "> ")) 
  {
    esp.print(text);
  } else 
  {
    dbg.println(F("closing"));
    esp.print( F("AT+CIPCLOSE=") );
    esp.println(ch_id);
  }
}

void serve_homepage(int ch_id, boolean SwitchRelay, boolean relayOn) 
{
  //String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nRefresh: 5\r\n";
  String header = F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n");

  String content="";
  if( SwitchRelay )
  {
    if( relayOn )
    {
      content += F("Switched relay ON <br/>\n");
    }
    else
    {
      content += F("Switched relay OFF <br/>\n");
    }    
  }
  else
  {
    // output the value of relay state
    content += F("Relay is ");
    if( GetRelayState() == HIGH )
    {
      content +=  F("ON");
    }
    else
    {
      content += F("OFF");
    }
    content += F(" <br/>\n");
  }

  header += F("Content-Length:");
  header += (int)(content.length());
  header += F("\r\n\r\n");
  esp.print( F("AT+CIPSEND=") );
  esp.print(ch_id);
  esp.print(",");
  esp.println(header.length()+content.length());
  if(wait_for_esp_response(2000, "> ")) 
  {
    esp.print(header);
    esp.print(content);
  } else 
  {
    dbg.println(F("closing"));
    esp.print( F("AT+CIPCLOSE=") );
    esp.println(ch_id);
  }
}


void SetupWiFiAsClient() 
{
  // try empty AT command
//  esp.println("AT");
//  wait_for_esp_response(1000);

  // set mode 1 (client)
  esp.println( F("AT+CWMODE=1") );
  wait_for_esp_response(1000);  

  // reset WiFi module
  esp.print( F("AT+RST\r\n") );
  wait_for_esp_response(1500);
  delay(3000);
 
  // join AP
  esp.print( F("AT+CWJAP=\"") );
//  esp.print(WIFI_SSID);
  esp.print( networkData.NetSSID );
  esp.print( F("\",\"") );
//  esp.print(PASS);
  esp.print( networkData.Password );
  esp.println("\"");
  // this may take a while, so wait for 5 seconds
  wait_for_esp_response(5000);
  
//  esp.println("AT+CIPSTO=30");  
//  wait_for_esp_response(1000);

  // start server
  esp.println( F("AT+CIPMUX=1") );
  wait_for_esp_response(1000);
  
  esp.print( F("AT+CIPSERVER=1,") ); // turn on TCP service
  esp.println(PORT);
  wait_for_esp_response(1000);
      
  // print firmware version
  dbg.print( F("Firmware: ") );
  esp.println( F("AT+GMR") );
  wait_for_esp_response(1000);
  
  // print device IP address
  dbg.print( F("device ip addr:") );
  esp.println( F("AT+CIFSR") );
  wait_for_esp_response(1000);
  
  // print device mac address
  dbg.print( F("device mac addr:") );
  esp.println( F("AT+CIPSTAMAC?") );
  wait_for_esp_response(1000);

  dbg.println( F("WiFi set as client finished") );
}

#define AP_SSID "HA_AP"
#define AP_PASS "1234"
#define AP_CHAN 11
void SetupWiFiAsAccessPoint() 
{
  // try empty AT command
//  esp.println("AT");
//  wait_for_esp_response(1000);

  // set mode 1 (client)
  esp.println( F("AT+CWMODE=2") );
  wait_for_esp_response(1000);  

  // reset WiFi module
  esp.print( F("AT+RST\r\n") );
  wait_for_esp_response(1500);
  delay(3000);
 
  esp.print( F("AT+CWSAP=\"") );
  esp.print( AP_SSID );
  esp.print( F("\",\"") );
  esp.print( AP_PASS );
//  esp.println( F("\"") );
  esp.print( F("\",") );
  esp.println( AP_CHAN );
//  esp.print( "," );
//  esp.println( AP_SEC );
  wait_for_esp_response(1000);

  esp.print( F("AT+RST\r\n") );
  wait_for_esp_response(1500);
  delay(3000);

  // start server
  esp.println( F("AT+CIPMUX=1") );
  wait_for_esp_response(1000);
  
  esp.print( F("AT+CIPSERVER=1,") ); // turn on TCP service
  esp.println(AP_PORT);
  wait_for_esp_response(1000);
      
  // print firmware version
  dbg.print( F("Firmware: " ));
  esp.println( F("AT+GMR") );
  wait_for_esp_response(1000);
  
  // print device IP address
  dbg.print( F("device ip addr:") );
  esp.println( F("AT+CIFSR") );
  wait_for_esp_response(1000);
  
//  esp.print( F("AT+RST\r\n") );
//  wait_for_esp_response(1500);
//  delay(3000);
  
  dbg.println( F("WiFi set as Access Point finished") );
}

boolean Bounce(int time, int lastCallTime, int debounceDelay )
{
  // The interrupt may be called multiple times on the same button press (or release)
  // So check only for real change
  int timelapse = time - lastCallTime;
//  dbg.print( "Time (millisecs) since last call: ");
//  dbg.println( timelapse );
  if( timelapse < debounceDelay )
  {
//    dbg.println( "Bounce detected, exiting..." );
    return 1;
  }
  dbg.print( F("Time (millisecs) since last call: ") );
  dbg.println( timelapse );
  return 0;
}

// Interrupt Service Routine (ISR)
void ButtonChange()
{
  dbg.println( F("ButtonChange...") );
  int time = millis();
  boolean bounce = Bounce( time, lastButtonCallTime, debounceButtonDelay) ;
  lastButtonCallTime = time;
  if( bounce )
  {
    return ;
  }
  
  int buttonState = digitalRead( BUTTON );
  dbg.print( F("BUTTON state: ") );
  dbg.println( buttonState );
  

  if( buttonState == LOW )
  {
    lastWifiMode = WifiMode;
    Timer1.stop();
    Timer1.initialize( WIFI_PROGRAMMING_DELAY ); //microsecs
    dbg.println( F("Started the timer") );
  }
  // every time the button is depressed (HIGH) toggle the led
  if( buttonState == HIGH )
  {
    Timer1.stop(); // stop the timer that has been started with a button press
    dbg.println( F("Stopped the timer") );
    dbg.print( F("WifiMode: ") );
    WifiMode == WIFI_WORKING ? dbg.print( F("working") ) : dbg.print( F("programming") );
    dbg.print( F(", lastMode: ") );
    lastWifiMode == WIFI_WORKING ? dbg.print( F("working") ) : dbg.print( F("programming") );
    dbg.println();
    if(  WifiMode == WIFI_WORKING && lastWifiMode == WIFI_WORKING )
    {
      ToggleRelay();
    }
    
    // no functionality yet for depressing the button in WIFI_PROGRAMMING mode
  }
}

void ToggleWifiMode()
{
  if( WifiMode == WIFI_PROGRAMMING )
  {
    WifiMode = WIFI_WORKING;
    dbg.println( F("WifiMode set to working") );
  }
  else
  {
    WifiMode = WIFI_PROGRAMMING;
    dbg.println( F("WifiMode set to programming") );
  }
  Timer1.stop();
}

void IRChange()
{
  dbg.println( F("IRChange...") );
  
  int time = millis();
  boolean bounce = Bounce( time, lastIRCallTime, debounceIRDelay) ;
  lastIRCallTime = time;
  if( bounce )
  {
    return ;
  }
  
  if( digitalRead( IR ) == LOW )
  {
    ToggleRelay();
  }
}

void OPTOChange()
{
  dbg.println( F("OPTOChange...") );
  
  int time = millis();
  boolean bounce = Bounce( time, lastOPTOCallTime, debounceOPTODelay) ;
  lastOPTOCallTime = time;
  if( bounce )
  {
    return ;
  }
  
  if( digitalRead( OPTO ) == LOW )
  {
    ToggleRelay();
  }
}

void SetupRelay() 
{
  pinMode(relayPin,OUTPUT);
//  SwitchRelay( false );
  digitalWrite(relayPin, GetRelayState());
  
}

void SwitchRelay( boolean on )
{
  if( on )
  {
    digitalWrite(relayPin, HIGH);
    SaveRelayState( HIGH );
  }
  else
  {
    digitalWrite(relayPin, LOW);
    SaveRelayState( LOW );
  }
}

void ToggleRelay()
{
//  int state = bitRead( PORTD,relayPin);
  int state = GetRelayState();
  dbg.print( F("State of pin ") );
  dbg.print( relayPin );
  dbg.print( F(" (relay): ") );
  dbg.println( state );
  if( state == HIGH )
  {
    dbg.println( F("Toggling relayPin to LOW") );
//    digitalWrite( relayPin, LOW );
//    SaveRelayState( LOW );
    SwitchRelay( false ); //off
  }
  else
  {
    dbg.println( F("Toggling relayPin to HIGH") );
//    digitalWrite( relayPin, HIGH );    
//    SaveRelayState( HIGH );
    SwitchRelay( true ); //on
  }
  ToggleLed();
}


//
//for debugging
//
void ToggleLed()
{
  dbg.println( F("Toggling the LED") );
//  int ledState = bitRead( PORTD,LED );
  int ledState = GetLedState();
  dbg.print( F("State of pin ") );
  dbg.print( LED );
  dbg.print( F(" (LED): ") );
  dbg.println( ledState );
  if( ledState == HIGH )
  {
    digitalWrite( LED, LOW );
    SaveLedState( LOW );
    dbg.println( F("The LED is now LOW") );
  }
  else
  {
    digitalWrite( LED, HIGH );    
    SaveLedState( HIGH );
    dbg.println( F("The LED is now HIGH") );
  }
}

// HIGH and LOW are integer constants see http://elabz.com/arduino-code-tidbits-1-declaring-an-array-with-pin-values/
void SaveLedState( int state )
{
  SaveState( LEDStateAddress, state );
}

void SaveRelayState( int state )
{
  SaveState( RelayStateAddress, state );
}

void SaveState( int address, int state )
{
  const byte high = 1;
  const byte low = 2;
  if( HIGH == state )
  {
    EEPROM.write( address, high );
  }
  else
  {
    EEPROM.write( address, low );
  }
}

int GetLedState()
{
  return GetState( LEDStateAddress );
}

int GetRelayState()
{
  return GetState( RelayStateAddress );
}

int GetState( int address )
{
  byte state = EEPROM.read( address );
  if( 1 == state )
  {
    return HIGH;
  }
  else
  {
    return LOW;
  }
}


//I don't know if this is a bug, but Timer1.start() and Timer1.restart() don't work
//I need to call Timer1.initialize every time for the Timer1 to run
void startTimer()
{
  Timer1.initialize(1000000);
}

void SampleUltra()
{
  dbg.println( F("Timer expired") );
  noInterrupts(); // disable interrupts while sampling the ultrasound sensor

  long avgDistance;  // centimeters
  for( int i = 0 ; i < UltraSampleNum ; i++ )
  {
    long duration, distance;
    digitalWrite(ULTRATRIG, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(ULTRATRIG, HIGH);
    //  delayMicroseconds(1000); - Removed this line
    delayMicroseconds(10); // Added this line
    digitalWrite(ULTRATRIG, LOW);
    duration = pulseIn(ULTRAECHO, HIGH);
    distance = (duration/2) / 29.1;  // centimeters
    ultraSamples[i] = distance;
    avgDistance += distance;
    delayMicroseconds( UltraSampleInterval );
  }
  avgDistance = avgDistance / UltraSampleNum;
  dbg.print( F("Avg distance: ") );
  dbg.print( avgDistance );
  if( avgDistance >= 200 || avgDistance <= 0 )
  {
    dbg.println( F(" cm. Out of range") );    
  }
  else 
  {
    dbg.println( F(" cm") );
  }

  if( avgDistance < ULTRA_DIST_THRESHHOLD ) 
  {  
    SwitchRelay( true );
  }
  else 
  {
    SwitchRelay( false );
  }
  
  interrupts();  //reenable interupts
}
