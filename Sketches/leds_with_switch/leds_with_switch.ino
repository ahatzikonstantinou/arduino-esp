#include "EEPROM.h"
#define UltraSampleNum 3  //how many samples to take every UltraSamplePeriod
#define UltraSampleInterval 1000  // microseconds to wait between each ultra sample
#define MaxUltraDistance 4000 // anything larger than this is considered an error and discarded
#define MinUltraDistance 0 // anything less than this is considered an error and discarded
int ultraSamples[UltraSampleNum];
#define ULTRAECHO 12
#define ULTRATRIG 11
#define ULTRA_DIST_PERSON 100  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
#define ULTRA_DIST_WAVE 30  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
#define PERSON_PRESENT_DELAY 2000UL // recheck after x millisenonds to ensure a person is still in front of the ultrasound sensor
#define PERSON_ABSENT_DELAY 4000UL // recheck after x millisenonds to ensure a person is still away from the ultrasound sensor


const int EEsize = 1024; // size in bytes of arduino UNO EEPROM
const int ModeAddress = 0;  //where the ledState is saved
const int DeviceNameAddress = 4;  //where the ledState is saved

// const byte WiFiLed = 6;
// const byte PersonLed = 7;
// const byte TwoWaveLed = 8;
// const byte OneWaveLed = 9;

//& segment display pin assignments
#define a7Seg 7
#define b7Seg 6
#define c7Seg A2 
#define d7Seg A1
#define e7Seg A0
#define f7Seg 8
#define g7Seg 9
#define dp7Seg A3

const byte BUTTON = 10;  // Button connected to digital pin 10
const byte relayPin = 5;
bool relayOn = false;

#define MODE_PERSON 0  //0: switch on when detect person
#define MODE_TWO_WAVE 1    //1: switch on when detect two gesture wave
#define MODE_ONE_WAVE 2    //2: switch on when detect single gesture wave
#define MODE_WIFI 3    //3: controlled by wifi only

const byte maxMode = MODE_WIFI;
volatile byte mode = 0;
                       
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

#define MIN_BUTTON_REPRESS 100UL // x msecs acceptable between 2 button presses
#define DEBOUNCE_DELAY 10UL // x msecs to guard against button debounce
volatile int lastCallTime = 0;
volatile int lastButtonState = 0;
#define BUTTON_SAMPLE_NUM 4

//Used for the WiFi
#define BUFFER_SIZE 512

String SSID =  "Wind WiFi 172839";      // change this to match your WiFi SSID
String PASS = "312ggp12";  // change this to match your WiFi password
String PORT = "80";           // using port 8080 by default
#define ESP_RESET_PIN 13
#define ESP_RESET_BUTTON 2

char buffer[BUFFER_SIZE];

#include <TimerOne.h>
int wifiFails = 0;
volatile bool wifiLedOn = false;
#define WIFI_MAX_SETUP_FAILS 3 //after n attempts to setup wifi, wifi is abandoned
#define WIFI_MAX_FAILS 1	// when maxFails occurs then special action is taken
#define WIFI_FAIL_BLINK_TIME_ON 500000 // blink on microsecs when blinking wifi leds
#define WIFI_FAIL_BLINK_TIME_OFF 200000 // blink off microsecs when blinking wifi leds
const String WIFI_IP_PATTERN = "192.168.";  //check for this pattern to determine if we are connected to the correct WIFI

//wifi states
#define WIFI_STATE_UNDEFINED 0
#define WIFI_STATE_NOT_SETUP 1
#define WIFI_STATE_SETTING_UP 2
#define WIFI_STATE_SETUP_FAILED 3
#define WIFI_STATE_OK 4
#define WIFI_STATE_ERROR 5
#define WIFI_STATE_LOG_OK 6 // set by DoLog
#define WIFI_STATE_LOG_SETTING_UP 7 // set by DoLog
#define WIFI_STATE_LOG_TRANSMITTING 8 // set by DoLog
#define WIFI_STATE_LOG_RESETTING_SERVER_MODE 9 // set by DoLog
byte wifiState = WIFI_STATE_NOT_SETUP; // set to WIFI_STATE_NOT_SETUP

//states during wifi setup returned by DoSetupWifi function only
#define DO_SETUPWIFI_WAITING 0
#define DO_SETUPWIFI_FAIL 1
#define DO_SETUPWIFI_SUCCESS 2

//wifi blinking
#define BLINK_WIFI_NONE -1  //do not blink
#define BLINK_WIFI_DP 0
#define BLINK_WIFI_BARS_SYNC 1
#define BLINK_WIFI_BARS_ASYNC 2
#define BLINK_WIFI_BARS_DP_SYNC 3
volatile byte wifiBlinkMode = BLINK_WIFI_NONE;
volatile bool wifiBlinkOn = false;

// vars and defines used by DoSetupWifi
byte stepAT = 0;                // currently running AT step in DoSetupWifi
String stepATSuccessTerm = "";  // term to look for in esp reponse so that RunStep knows it was successfull
String stepATErrorTerm = "";    // term to look for in esp reponse so that RunStep knows it was error
String espString = "";         // a string to hold incoming data from esp

//values set in var stepATTermSearch, used by ReadEspSerial
#define STEP_AT_TERM_SEARCHING 0
#define STEP_AT_SUCCESS_TERM_FOUND 1
#define STEP_AT_ERROR_TERM_FOUND 2
#define STEP_AT_TERM_NOT_FOUND 3
volatile byte stepATTermSearch = STEP_AT_TERM_SEARCHING;

// values returned by RunStep
#define STEP_AT_RUN_NOT_CURRENT -1
#define STEP_AT_RUN_RUNNING 0
#define STEP_AT_RUN_FAIL 1
#define STEP_AT_RUN_SUCCESS 2
#define STEP_AT_RUN_TIMEDOUT 3

#define STEP_AT_RUN_STATUS_RUNNING 0
#define STEP_AT_RUN_STATUS_IDLE 1
byte stepATRunStatus = STEP_AT_RUN_STATUS_IDLE;

#define ESP_REQUEST_NONE 0
#define ESP_REQUEST_NEW 1
struct EspRequest
{
  String ch_id;
  String text;
  byte status;
} espRequest; // global var to hold the current status of esp incoming requests

#define ESP_SEND_NO_SEND 0
#define ESP_SEND_SEND 1
struct EspResponse
{
  byte status; //ESP_SEND_NO_SEND or ESP_SEND_SEND
  String header;
  String content;
} espResponse;

#define LOG_SERVER_TCP_PORT "8000"
#define LOG_SERVER_IP_ADDRESS "192.168.1.4"

#define LOG_ACTOR_WIFI 0
#define LOG_ACTOR_BUTTON 1
#define LOG_ACTOR_ULTRASOUND 2
#define LOG_ACTOR_SYSTEM 3
#define LOG_TARGET_RELAY 0
#define LOG_TARGET_MODE 1
#define LOG_TARGET_NAME 2
#define LOG_TARGET_SYSTEM 3
typedef struct ALogData
{
  byte empty: 1;  // if 1 this record may be considered empty and replaced. Set to 0 so that
                  // the record will be transmitted to the log server
                  // Set to 1 after transmission so that this place may be re-used
  byte actor; // who did the action, LOG_ACTOR_WIFI, ...
  byte target;
  unsigned long time; // when the action happened in msecs
  String value; // the new value e.g. "on" for relay, "one_wave" for mode, etc.  
} LogData;
#define LOG_DATA_ARRAY_SIZE 10
LogData logData[LOG_DATA_ARRAY_SIZE];
#define LOG_MIN_TIMESPAN 4000 // will transmit log only every LOG_MIN_TIMESPAN millisecs or longer

#define dbg Serial
// #define dbg esp

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
                                 // remember that every time you read inputString
                                 // you must set stringComplete back to false
String EspString = "";      // this string holds a complete esp sentence

#define DEVICE_NAME_SIZE 30
char DeviceName[DEVICE_NAME_SIZE] = "cupboard-multi-switch";

#include <SoftwareSerial.h>
SoftwareSerial esp(4,3);  // (Rx,Tx) use pins 7, 8 for software serial 
// SoftwareSerial dbg(4,3);  // (Rx,Tx) use pins 7, 8 for software serial 

#define ESP_RESPONSE_NOTREADYYET 0
#define ESP_TERM_NOTFOUND 1
#define ESP_TERM_FOUND 2

// // By default we are looking for OK\r\n

void setup() 
{
  esp.begin(9600); // 9600 57600 115200
  
  dbg.begin(9600);
  dbg.println( F("-----------------------"));
  dbg.println(F("begin."));
 
  pinMode(relayPin,OUTPUT);

  pinMode( a7Seg, OUTPUT );
  pinMode( b7Seg, OUTPUT );
  pinMode( c7Seg, OUTPUT );
  pinMode( d7Seg, OUTPUT );
  pinMode( e7Seg, OUTPUT );
  pinMode( f7Seg, OUTPUT );
  pinMode( g7Seg, OUTPUT );
  pinMode( dp7Seg, OUTPUT );
  TurnLedsOff();

  // setup ultrasound sensor
  pinMode( ULTRATRIG, OUTPUT );
  pinMode( ULTRAECHO, INPUT );
  
  // the button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON, INPUT_PULLUP );

  wifiState = WIFI_STATE_NOT_SETUP;

  // for( int i = 0 ; i < DEVICE_NAME_SIZE ; i++ )
  // {
  //   DeviceName[i] = EEPROM.read( DeviceNameAddress+i ); // initialise from eprom
  // }

  mode = EEPROM.read( ModeAddress ); // initialise from eprom
  dbg.print( F("Ard: mode read from EEPROM: ") );
  dbg.println( mode );
  if( mode > maxMode )
  {
    mode = MODE_PERSON;
    dbg.println( F("Ard: will set mode to MODE_PERSON.") );
  }
  dbgMode();
  SetLeds();

	pinMode( ESP_RESET_PIN, OUTPUT );
	digitalWrite( ESP_RESET_PIN, HIGH);
  pinMode( ESP_RESET_BUTTON, INPUT_PULLUP );

  // reserve x bytes for the inputString:
  //inputString.reserve(500);

  espRequest.status = ESP_REQUEST_NONE;
  espResponse.status = ESP_SEND_NO_SEND;

  for( int i = 0 ; i < LOG_DATA_ARRAY_SIZE ; i++ )
  {
    logData[i].empty = 1;
  }

  // first log entry
  Log( LOG_ACTOR_SYSTEM, LOG_TARGET_SYSTEM, "system started" );
  // ALogData data1 = { 0, LOG_ACTOR_SYSTEM, LOG_TARGET_SYSTEM, millis(), "system started" };
  // AddLogData( data1 ); 
  // ALogData data2 = { 0, LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, millis() + 10, "on" };
  // AddLogData( data2 ); 
}

// bool done = false;
void loop() 
{
  // dbg.println( "loop") ;

  DoSetupWiFi();
  
  ReadEspSerial();
  
  //SetupWiFi();
  // if( !done )
  // {
  //   byte aux = DO_SETUPWIFI_WAITING;
  //   while( aux == DO_SETUPWIFI_WAITING )
  //   {
  //     dbg.println( "waiting DoSetupWifi" );
  //     aux = DoSetupWiFi();
  //   }
  //   if( aux = DO_SETUPWIFI_FAIL )
  //   {
  //     dbg.println( "DoSetupWifi failed" );
  //   }
  //   else
  //   {
  //     dbg.println( "DoSetupWifi success" );
  //   }
  //   done = true;
  // }
  
  DoWiFi();  // we always read wifi to be able to set mode from wifi no matter what current mode is

  ReadButton();

  if( mode != MODE_WIFI )
  {
    if( mode == MODE_PERSON )
    {
      ////dbg.println( "mode person" );
      if( !relayOn && DetectPersonPresent() == DetectPersonStatusIsPresent )
      {
        Switch( true );
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, "on" );
        //dbg.println( "Turned relay ON" );
      }
      else if( relayOn && DetectPersonAbsent() == DetectPersonStatusIsAbsent )
      {
        Switch( false );
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, "off" );
        //dbg.println( "Turned relay OFF" );
      }
    }
    else if( mode == MODE_TWO_WAVE )
    {
      ////dbg.println( "mode wave" );
      if( DetectTwoWave() == TwoWaveDetected )
      {
        Toggle();
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, ( relayOn ? "on" : "off" ) );
      }
    }
    else if( mode == MODE_ONE_WAVE )
    {
      ////dbg.println( "mode wave" );
      if( DetectOneWave() == OneWaveDetected )
      {
        Toggle();
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, ( relayOn ? "on" : "off" ) );
      }
    }
  }

  DoEspSend(); // this call needs to be here in order to finish sends. EspSend has 2 AT steps

  DoLog(); // this call needs to be here in order to send log updates to log server

}

void Toggle()
{
  if( relayOn )
  {
    Switch( false );
  }
  else
  {
    Switch( true );
  }
}

void Switch( bool on )
{
  relayOn = on;
  digitalWrite(relayPin, on ? HIGH : LOW );  
  //dbg.print( "Turned relay " ); 
  //on ? dbg.println( "ON" ) : dbg.println( "OFF" );
}

void DoWiFi()
{
  if( espRequest.status == ESP_REQUEST_NEW )
  {
    if( espRequest.text.indexOf( "GET /on " ) > -1 )
    {
      dbg.println( F("Ard: Will switch ON."));
      Switch( true );
      EspSendState();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_RELAY, "on" );
    }
    else if( espRequest.text.indexOf( "GET /off " ) > -1 )
    {
      dbg.println( F("Ard: Will switch OFF."));
      Switch( false );
      EspSendState();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_RELAY, "off" );
    }
    else if( espRequest.text.indexOf( "GET /person " ) > -1 )
    {
      dbg.println( F("Ard: Will set mode to PERSON."));
      ChangeMode( MODE_PERSON );
      EspSendMode();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "person" );
    }
    else if( espRequest.text.indexOf( "GET /one_wave " ) > -1 )
    {
      dbg.println( F("Ard: Will set mode to ONE WAVE."));
      ChangeMode( MODE_ONE_WAVE );
      // Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "one_wave" );
      EspSendMode();
    }
    else if( espRequest.text.indexOf( "GET /two_wave " ) > -1 )
    {
      dbg.println( F("Ard: Will set mode to TWO WAVE."));
      ChangeMode( MODE_TWO_WAVE );
      EspSendMode();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "two_wave" );
    }
    else if( espRequest.text.indexOf( "GET /wifi " ) > -1 )
    {
      dbg.println( F("Ard: Will set mode to WIFI."));
      ChangeMode( MODE_WIFI );
      EspSendMode();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "wifi" );
    }
    else if( espRequest.text.indexOf( "GET /mode " ) > -1 )
    {
      dbg.println( F("Ard: Will return mode."));
      EspSendMode();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "mode_query" );
    }
    else if( espRequest.text.indexOf( "GET /state " ) > -1 )
    {
      dbg.println( F("Ard: Will return state (ON/OFF)."));
      EspSendState();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "state_query" );
    }
    else if( espRequest.text.indexOf( "GET /status " ) > -1 )
    {
      dbg.println( F("Ard: Will return status (mode and state)."));
      EspSendStatus();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "status_query" );
    }
    else if( espRequest.text.indexOf( "GET /name " ) > -1 )
    {
      dbg.println( F("Ard: Will return name."));
      EspSendName();
      Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "name_query" );
    }
    else if( espRequest.text.indexOf( "GET /name=" ) > -1 )
    {
      int ind = espRequest.text.indexOf( "GET /name=" );
      int end = espRequest.text.indexOf( " ", ind + 1 );
      String name = DeviceName;
      if( end > -1 )
      {
        name = espRequest.text.substring( ind, end );
      }
      else
      {
        name = espRequest.text.substring( ind); 
      }
      dbg.print( F("Ard: Will set name to [") );
      dbg.print( name );
      dbg.println( F("].") );
      UpdateDeviceName( name );
      Log( LOG_ACTOR_WIFI, LOG_TARGET_NAME, "name" );
    }
  }
  espRequest.status = ESP_REQUEST_NONE; // reset to stop executing the request continuously
}

// unsigned long BlinkWifiTimer = 0UL;
void BlinkWifiLed()
{
  // if( blinkMode = BLINK_WIFI_NONE )
  // {
  //   return;
  // }

  // unsigned long t = millis();
  if( wifiBlinkOn )
  {
    // if( t - BlinkWifiTimer < WIFI_FAIL_BLINK_TIME_OFF )
    // {
    //   return;
    // }
   Timer1.initialize( WIFI_FAIL_BLINK_TIME_OFF );
  }
  else
  {
    // if( t - BlinkWifiTimer < WIFI_FAIL_BLINK_TIME_ON )
    // {
    //   return;
    // }
   Timer1.initialize( WIFI_FAIL_BLINK_TIME_ON );
  }

  // BlinkWifiTimer = t;
  switch( wifiBlinkMode )
  {
    case BLINK_WIFI_DP:
      digitalWrite( dp7Seg, wifiBlinkOn ? LOW : HIGH );
      break;
    case BLINK_WIFI_BARS_SYNC:
      digitalWrite( b7Seg, wifiBlinkOn ? LOW : HIGH );
      digitalWrite( c7Seg, wifiBlinkOn ? LOW : HIGH );
      break;
    case BLINK_WIFI_BARS_ASYNC:
      digitalWrite( b7Seg, wifiBlinkOn ? HIGH : LOW );
      digitalWrite( c7Seg, wifiBlinkOn ? HIGH : HIGH );
      break;
    case BLINK_WIFI_BARS_DP_SYNC:
      digitalWrite( b7Seg, wifiBlinkOn ? LOW : HIGH );
      digitalWrite( c7Seg, wifiBlinkOn ? LOW : HIGH );
      digitalWrite( dp7Seg, wifiBlinkOn ? LOW : HIGH );
      break;
    default:
      break;
  }
  wifiBlinkOn = !wifiBlinkOn;
  Timer1.attachInterrupt( BlinkWifiLed );
}

void BlinkWifiLedStart( byte blinkMode )
{
  wifiBlinkMode = blinkMode;
  BlinkWifiLed();
}

void BlinkWifiLedStop()
{
  // blinkMode = BLINK_WIFI_NONE;
  Timer1.detachInterrupt();
}

void ESPFail()
{
	BlinkWifiLed();
	wifiFails++;
	if( wifiFails >= WIFI_MAX_FAILS )
	{
		ESPReset();
	}
}

void ESPReset()
{
	digitalWrite( ESP_RESET_PIN, LOW );
	delay( 100 );
	digitalWrite( ESP_RESET_PIN, HIGH );
}
/*
void SetupWiFi()
{
  // //dbg.print( "wifiState:" );
  // //dbg.println( wifiState ) ;
  //run only when state is WIFI_STATE_NOT_SETUP
  if( wifiState != WIFI_STATE_NOT_SETUP )
  {
    // //dbg.println( "no need to setup wifi" ) ;
    return;
  }

  dbg.println( "starting wifi setup" ) ;
  wifiState = WIFI_STATE_SETTING_UP;
  SetLedsWifi();
  bool setupWifiSuccess = false;
  for( int w=0 ; w < WIFI_MAX_SETUP_FAILS && !setupWifiSuccess; w++ )
  {
    dbg.print( w );
    dbg.println( " trying DoSetupWiFi");
    dbg.println( "Resetting esp..." );
    digitalWrite( ESP_RESET_PIN, LOW );
    dbg.println( "waiting 10 msecs" );
    delay( 10 );
    dbg.println( "Restoring esp..." );
    digitalWrite( ESP_RESET_PIN, HIGH );
    dbg.println( "esp should be restored now" );
    setupWifiSuccess = DoSetupWiFi();
    if( setupWifiSuccess )
    {
      break;
    }
    else
    {
      dbg.println( "Something went wrong, will wait to try DoSetupWiFi again.");
      delay( 3000 );
    }
  }
  if( !setupWifiSuccess )
  {
    //permanent wifi failure
    dbg.println( "Permanent wifi failure." );
    wifiState = WIFI_STATE_SETUP_FAILED;
  }
  else
  {
    dbg.println( "WiFi intialised successfully." );
    wifiState = WIFI_STATE_OK;
  }
  SetLedsWifi();
}
*/

unsigned long PersonPresentTimer = 0UL;
bool PersonPresentStarted = false;
// This function will wait for a while in case person moves quickly in front of the ultrasound sensor
byte DetectPersonPresent()
{
  if( !PersonPresentStarted )
  {
    PersonPresentTimer = millis();
    PersonPresentStarted = true;
  
  	long distance = SampleUltra();
    if( distance < ULTRA_DIST_PERSON  && distance > 0 )
    {
  		//dbg.print( "start of person-present distance: " );
      //dbg.print( distance );
      //dbg.println(" cm");
      return DetectStatusNotReadyYet;
    }
  }
  else
  {
    unsigned long elapsedTime = millis() - PersonPresentTimer;
    // //dbg.print( "elapsedTime: ");
    // //dbg.print( elapsedTime );
    // //dbg.println( " msecs" );
    if( elapsedTime < PERSON_PRESENT_DELAY )
    {
      // //dbg.println("DetectPersonPresent still checking");
      return DetectStatusNotReadyYet;
    }

		long distance = SampleUltra();
		//dbg.print( "end of person-present distance: " );
    //dbg.print( distance );
    //dbg.println(" cm");
    if( distance < ULTRA_DIST_PERSON  && distance > 0 )
    {
      //dbg.println("Person is still here.");
      PersonPresentStarted = false;
      return DetectPersonStatusIsPresent;
    }
	}
  PersonPresentStarted = false;
	return DetectPersonStatusIsAbsent;
}

unsigned long PersonAbsentTimer = 0UL;
bool PersonAbsentStarted = false;
// This function will wait for a while in case person moves quickly in front of the ultrasound sensor
byte DetectPersonAbsent()
{
  if( !PersonAbsentStarted )
  {
    PersonAbsentTimer = millis();
    PersonAbsentStarted = true;

    long distance = SampleUltra();
    if( distance > ULTRA_DIST_PERSON || distance < 0 )
    {
  		//dbg.print( "start of person-absent distance: " );
      //dbg.print( distance );
      //dbg.println(" cm");
      return DetectStatusNotReadyYet;
    }
  }
  else
  {
    unsigned long elapsedTime = millis() - PersonAbsentTimer;
    // //dbg.print( "elapsedTime: ");
    // //dbg.print( elapsedTime );
    // //dbg.println( " msecs" );
    if( elapsedTime < PERSON_ABSENT_DELAY )
    {
      // //dbg.println("DetectPersonAbsent still checking");
      return DetectStatusNotReadyYet;
    }
    
		long distance = SampleUltra();
		//dbg.print( "end of person-absent distance: " );
    //dbg.print( distance );
    //dbg.println(" cm");
    if( distance > ULTRA_DIST_PERSON || distance < 0 )
    {
      //dbg.println("Person is still absent.");
      PersonAbsentStarted = false;
      return DetectPersonStatusIsAbsent;
    }
	}
  PersonAbsentStarted = false;
	return DetectPersonStatusIsPresent;
}

unsigned long OneWaveTimer = 0UL;
bool OneWaveStarted = false;
byte DetectOneWave()
{
  if( !OneWaveStarted )
  {
    long distance = SampleUltra();
    if( distance < ULTRA_DIST_WAVE  && distance > 0 )
    {
      OneWaveTimer = millis();
      OneWaveStarted = true;
      //dbg.print( "start of one-wave distance: " );
      //dbg.print( distance );
      //dbg.println(" cm");
      return DetectStatusNotReadyYet;
    }
  }
  else
  {
    if( millis() - OneWaveTimer < ONE_WAVE_DELAY )
    {
      return DetectStatusNotReadyYet;
    }
    
    long distance = SampleUltra();
    //dbg.print( "end of one-wave distance: " );
    //dbg.print( distance );
    //dbg.println(" cm");
    // must move hand out of utlrasound's range
    if( distance < ULTRA_DIST_WAVE && distance > 0 )
    {
      //dbg.println("No one-wave. Obstacle still in the way.");
      OneWaveStarted = false;
      return OneWaveNotDetected;
    }
    else
    {
      OneWaveStarted = false;
      return OneWaveDetected;
    }
	}
  OneWaveStarted = false;
	return OneWaveNotDetected;
}

unsigned long TwoWaveTimer = 0UL;
bool TwoWaveStarted = false;
bool TwoWaveMiddle = false;
byte DetectTwoWave()
{
  if( !TwoWaveStarted )
  {
    long distance = SampleUltra();
    if( distance < ULTRA_DIST_WAVE  && distance > 0 )
    {
      TwoWaveTimer = millis();
      TwoWaveStarted = true;
      //dbg.print( "start of two-wave distance: " );
      //dbg.print( distance );
      //dbg.println(" cm");
      return DetectStatusNotReadyYet;
    }
  }
  else if( TwoWaveStarted && !TwoWaveMiddle )
  {    
    if( millis() - TwoWaveTimer < TWO_WAVE_DELAY )
    {
      return DetectStatusNotReadyYet;
    }
    
    long distance = SampleUltra();
    //dbg.print( "middle of two-wave distance: " );
    //dbg.print( distance );
    //dbg.println(" cm");
    // if hand out of utlrasound's range we are ready for the second and final wave
    if( distance > ULTRA_DIST_WAVE || distance < 0 )
    {
      TwoWaveMiddle = true;     // finished with middle of gesture
      TwoWaveTimer = millis();  // reset for final countdown
      return DetectStatusNotReadyYet;
    }
    else
    {
      //dbg.println("No middle of two-wave found. Middle of wave delay exhausted");
      TwoWaveStarted = false;
      TwoWaveMiddle = false;
      return TwoWaveNotDetected;
    }
  }
  else if( TwoWaveStarted && TwoWaveMiddle )
  {
    // this is the final countdown, the second gesture of the two wave gestures
    if( millis() - TwoWaveTimer > TWO_WAVE_DURATION )
    {
      //dbg.println("No end of two-wave found. Wave duration exhausted");
      TwoWaveStarted = false;
      TwoWaveMiddle = false;
      return TwoWaveNotDetected;
    }

    long distance = SampleUltra();
    // //dbg.print( "looking for end of two-wave distance: " );
    // //dbg.print( distance );
    // //dbg.println(" cm");
    if( distance < ULTRA_DIST_WAVE && distance > 0 )
    {
      //dbg.print("End of two-wave found, distance: ");
      //dbg.println( distance );
      TwoWaveStarted = false;
      TwoWaveMiddle = false;
      return TwoWaveDetected;
    }
    else
    {
      // //dbg.println("Hand not waved in yet. Waiting for second wave gesture/obstacle.");
      return DetectStatusNotReadyYet; 
    }
  }
  TwoWaveStarted = false;
  TwoWaveMiddle = false;
  return TwoWaveNotDetected;
}

int SampleUltra()
{
    long duration = 0;
    long distance = -1;
    while( distance < MinUltraDistance || distance > MaxUltraDistance )
    {
      digitalWrite(ULTRATRIG, LOW);  // Added this line
      delayMicroseconds(10); // Added this line
      digitalWrite(ULTRATRIG, HIGH);
      //  delayMicroseconds(1000); - Removed this line
      delayMicroseconds(10); // Added this line
      digitalWrite(ULTRATRIG, LOW);
      duration = pulseIn(ULTRAECHO, HIGH);
      distance = (duration/2) / 29.1;  // centimeters
    }
    return distance;
}

int SampleUltraMulti()
{
  long avgDistance = 0;  // centimeters
  long totalDistance = 0;
  long validSampleCount = 0;
  for( int i = 0 ; i < UltraSampleNum ; i++ )
  {
    long distance = 0;//GetUltraDistance();
    if( distance < 200 && distance > 0 )
    {
//      ultraSamples[i] = distance;
      avgDistance += distance;
      validSampleCount++;
    }
    delayMicroseconds( UltraSampleInterval );
  }
//  //dbg.print( "Total distance: " );
//  //dbg.println( totalDistance );
  avgDistance = avgDistance / validSampleCount;
//  //dbg.print( "Avg distance: " );
//  //dbg.print( avgDistance );
//  //dbg.println(" cm");
  return avgDistance;
  
  //if( avgDistance >= 200 || avgDistance <= 0 )
  //{
    ////dbg.println(" cm. Out of range");    
  //}
  //else 
  //{
    ////dbg.println(" cm");
  //}

  //if( avgDistance > 0 && avgDistance < ULTRA_DIST_THRESHHOLD ) 
  //{  
    //SwitchRelay( true );
  //}
  //else 
  //{
    //SwitchRelay( false );
  //}
  
}

void TurnLedsOff()
{
  digitalWrite( a7Seg,  HIGH );
  digitalWrite( b7Seg,  HIGH );
  digitalWrite( c7Seg,  HIGH );
  digitalWrite( d7Seg,  HIGH );
  digitalWrite( e7Seg,  HIGH );
  digitalWrite( f7Seg,  HIGH );
  digitalWrite( g7Seg,  HIGH );
  digitalWrite( dp7Seg, HIGH );
}

void SetLedsPerson()
{
  // digitalWrite( a7Seg, LOW );
  digitalWrite( e7Seg, LOW );
  digitalWrite( f7Seg, LOW );
}

void SetLedsOneWave()
{
  digitalWrite( g7Seg, LOW );
}

void SetLedsTwoWave()
{
  digitalWrite( g7Seg, LOW );
  digitalWrite( d7Seg, LOW ); 
}

void SetLedsWifi()
{
  // //dbg.print( "In SetLedsWifi wifiState: ");
  // //dbg.println( wifiState );
  switch( wifiState )
  {
    case WIFI_STATE_UNDEFINED:
      // //dbg.println( "blinking BLINK_WIFI_BARS_ASYNC." );
      BlinkWifiLedStart( BLINK_WIFI_BARS_ASYNC );
      break;
    case WIFI_STATE_SETTING_UP:
      // //dbg.println( "blinking BLINK_WIFI_BARS_SYNC." );
      BlinkWifiLedStart( BLINK_WIFI_BARS_SYNC );
      break;
    case WIFI_STATE_SETUP_FAILED:
      digitalWrite( b7Seg, HIGH );
      digitalWrite( c7Seg, HIGH );
      // //dbg.println( "blinking BLINK_WIFI_DP." );
      BlinkWifiLedStart( BLINK_WIFI_DP );
      break;
    case WIFI_STATE_OK:
      // //dbg.println( "Stopped blinking." );
      BlinkWifiLedStop();
      digitalWrite( b7Seg, LOW );
      digitalWrite( c7Seg, LOW );
      break;
    case WIFI_STATE_ERROR:
      digitalWrite( b7Seg, LOW );
      digitalWrite( c7Seg, LOW );
      // //dbg.println( "blinking BLINK_WIFI_DP." );
      BlinkWifiLedStart( BLINK_WIFI_DP );
      break;
    default:
      break;
  }  
}

void SetLeds()
{
  TurnLedsOff();
  if( mode == MODE_PERSON )
  {
    SetLedsPerson();
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    SetLedsTwoWave();
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    SetLedsOneWave();
  }

  //WIFI is always on (depending on its state)
  SetLedsWifi();
}

unsigned long ButtonDebounceTimer = 0UL;
bool DebounceButton = false;
void ReadButton()
{
  int buttonState = digitalRead( BUTTON );
  if( !DebounceButton )
  {
    unsigned long elapsedTime = millis() - lastCallTime;
    if( elapsedTime > MIN_BUTTON_REPRESS )
    {
      if( buttonState == HIGH && buttonState != lastButtonState )
      {
        // //dbg.print( "Checking button after ");
        // //dbg.print( elapsedTime );
        // //dbg.println( " milliseconds" );
        // //dbg.print( "elapsedTime is valid ( > ");
        // //dbg.print( MIN_BUTTON_REPRESS );
        // //dbg.println( " )");
        // //dbg.println( "button was low and now is high. Will debounce." );
        lastCallTime = millis();
        DebounceButton = true;
      }

      lastButtonState = buttonState;
    }
  }
  else
  {
    //wait until we are past the DEBOUNCE_DELAY
    if( millis() - lastCallTime > DEBOUNCE_DELAY )
    {
      // //dbg.println( "Debouncing button..." );
      DebounceButton = false;
      int highCount = 0;
      //get a few samples to avoid false positives or false negatives
      for( int i = 0 ; i < BUTTON_SAMPLE_NUM ; i++ )
      {
        if( digitalRead(BUTTON) == HIGH )
        {
          highCount++;
        }        
      }

      // every time the button is depressed (HIGH) toggle the led
      if( highCount > ( BUTTON_SAMPLE_NUM/2 ) )
      {
        GoToNextMode();
      }
    }
  }
}

void GoToNextMode()
{
  if( mode == maxMode )
  {
    ChangeMode( 0 );
  }
  else
  {
    ChangeMode( mode + 1 );
  }
}

void dbgMode()
{
  if( mode == MODE_PERSON )
  {
    //dbg.println( "mode person" );
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    //dbg.println( "mode two-wave" );
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    //dbg.println( "mode one-wave" );
  }
  else if ( mode == MODE_WIFI )
  {
    //dbg.println( "mode wifi" );
  }
}

void ChangeMode( int newMode )
{
  if( mode < 0 || mode > maxMode )
  {
    //dbg.print( "Error. Cannot change mode to " );
    //dbg.print( newMode );
    //dbg.print( ". Mode must be a number between 0 and " );
    //dbg.print( maxMode );
    //dbg.print( ". Mode will not be changed!" );
  }
  
  mode = newMode ;
  dbgMode();
  EEPROM.put( ModeAddress, mode ); // save to eprom
  
  SetLeds();
}

void UpdateDeviceName( String name )
{
  // if( name.length() > 0 )
  // {
  //   name.toCharArray( DeviceName, DEVICE_NAME_SIZE );
  // }
}

/////////////////
// WIFI staff
/////////////////
byte previousStepResult = STEP_AT_RUN_NOT_CURRENT; // set only when a step finishes
byte PreviousStepResult( int currentStep )
{
  if( stepAT != currentStep )
  {
    return STEP_AT_RUN_NOT_CURRENT;
  }

  return previousStepResult;
}

bool stepStarted = false;
unsigned long stepATTimer = 0UL;
byte RunStep( byte step, String cmd, unsigned long timeout, 
              String successTerm = "OK", String errorTerm = "ERROR" )
{
  // dbg.print( "runstep in step ") ; dbg.println( step );
  if( stepAT != step ) 
  {
    return STEP_AT_RUN_NOT_CURRENT;
  }

  if( !stepStarted && stepATRunStatus != STEP_AT_RUN_RUNNING )
  {
    dbg.print( F("Ard: Started step (") );
    dbg.print( step );
    dbg.print( F(") cmd: {") );
    dbg.print( cmd );
    dbg.print( F("} ending with successTerm [") );
    dbg.print( successTerm );
    dbg.print( F("], errorTerm: [") );
    dbg.print( errorTerm );
    dbg.println( F("]." ) );
    stepStarted = true;
    stepATTimer = millis();
    if( cmd.length() > 0 )
    {
      esp.println( cmd );
    }
    stepATTermSearch = STEP_AT_TERM_SEARCHING;
    stepATSuccessTerm = successTerm;
    stepATErrorTerm = errorTerm;
    stepATRunStatus = STEP_AT_RUN_STATUS_RUNNING;
    return STEP_AT_RUN_RUNNING;
  }

  if( stepATTermSearch == STEP_AT_TERM_SEARCHING )
  {
    unsigned long elapsedTime = millis() - stepATTimer;
    if( elapsedTime > timeout )
    {
      stepAT = stepAT+1;
      stepStarted = false;
      dbg.print( F("Ard: cmd [") );
      dbg.print( cmd );
      dbg.print( F("] timed out after ") );
      dbg.print( elapsedTime );
      dbg.print( F(" milliseconds, timeout: ") );
      dbg.print( timeout );
      dbg.print( F(" (next step is ") );
      dbg.print( stepAT );
      dbg.println( F(").") );
      previousStepResult = STEP_AT_RUN_TIMEDOUT;
      stepATRunStatus = STEP_AT_RUN_STATUS_IDLE;
      return STEP_AT_RUN_TIMEDOUT;
    }
    // dbg.println( "Still running" );
    return STEP_AT_RUN_RUNNING;
  }

  stepATSuccessTerm = "";
  stepATErrorTerm = "";
  stepAT = stepAT+1;
  stepStarted = false;
  stepATRunStatus = STEP_AT_RUN_STATUS_IDLE;
  if( stepATTermSearch == STEP_AT_TERM_NOT_FOUND )
  {
    dbg.print( F("Ard: cmd [") );
    dbg.print( cmd );
    dbg.print( F("] failed (next step is ") );
    dbg.print( stepAT );
    dbg.println( F(").") );
    previousStepResult = STEP_AT_RUN_FAIL;
    return STEP_AT_RUN_FAIL;
  }
  else if( stepATTermSearch == STEP_AT_SUCCESS_TERM_FOUND )
  {
    dbg.print( F("Ard: Success (next step is ") );
    dbg.print( stepAT );
    dbg.println( F(").") );
    previousStepResult = STEP_AT_RUN_SUCCESS;  
    return STEP_AT_RUN_SUCCESS;
  }

  dbg.print( F("Ard: Error (next step is ") );
  dbg.print( stepAT );
  dbg.println( F(").") );
  previousStepResult = STEP_AT_RUN_FAIL;  
  return STEP_AT_RUN_FAIL;
}

unsigned long DoSetupWiFiTimer = 0UL;
bool DoSetupWiFiStarted = false;
byte DoSetupWifiRetries = 0;
void DoSetupWiFi() 
{  
  if( wifiState != WIFI_STATE_NOT_SETUP && wifiState != WIFI_STATE_SETTING_UP )
  {
    return;
  }

  if( !DoSetupWiFiStarted )
  {
    DoSetupWifiRetries = 0;
    stepAT = 0;
    DoSetupWiFiTimer = millis();
    DoSetupWiFiStarted = true;
    ESPReset();
    // while( esp.available() ); // eat leftover bytes lingering in serial, before issuing any AT commands
    dbg.println( F("Ard: starting wifi setup") ) ;
    wifiState = WIFI_STATE_SETTING_UP;
    SetLedsWifi();
  }

  // if( DoSetupWifiRetries > WIFI_MAX_SETUP_FAILS )
  // {
  //   dbg.print( F("Ard: reachde WIFI_MAX_SETUP_FAILS (") );
  //   dbg.print( WIFI_MAX_SETUP_FAILS );
  //   dbg.print( F(") after ") );
  //   dbg.print( ( millis() - DoSetupWiFiTimer ) );
  //   dbg.println( F("milliseconds.") );
  //   DoSetupWiFiStarted = false;
  //   dbg.println( F("Ard: Permanent wifi failure.") );
  //   wifiState = WIFI_STATE_SETUP_FAILED;
  //   return;
  // }

  byte lastStepResult = -1;
  // sometimes esp reconnects on its own, using the last known wifi connection
  lastStepResult = RunStep( 0, "", 10000UL, "WIFI GOT IP|OK", "ERROR" );
  if( PreviousStepResult( 1 ) == STEP_AT_RUN_SUCCESS )
  {
    dbg.println( F("Ard: DoSetupWiFi started with WIFI GOT IP. Will check if IP = '192.168.'") );
    lastStepResult = RunStep( 1, "AT+CIFSR", 3000UL, WIFI_IP_PATTERN + "|OK", "ERROR" );
  }
  else if( PreviousStepResult( 1 ) == STEP_AT_RUN_FAIL || PreviousStepResult( 1 ) == STEP_AT_RUN_TIMEDOUT )
  {
    stepAT = 2;
  }

  if( PreviousStepResult( 2 ) == STEP_AT_RUN_SUCCESS ) // step 1 finished successfully
  {
    dbg.println( F("Ard: DoSetupWiFi reconnected immediately.") );
    stepAT = 5; //go to step AT+CIPMUX=1
  }
  while( true )
  {
    // esp did not start by reconnecting to last known wifi, must setup
    lastStepResult = RunStep( 2, "AT", 15000UL );
    if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
    lastStepResult = RunStep( 3, "AT+CWMODE=1", 10000UL );
    if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
    // RunStep( 3, "", 5000UL, "" ); //just wait for 5000 millisecs
    lastStepResult = RunStep( 4, "AT+CWJAP=\"" + SSID + "\",\"" + PASS + "\"", 15000UL );
    if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
    lastStepResult = RunStep( 5, "AT+CIPMUX=1", 10000UL );
    if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
    lastStepResult = RunStep( 6, "AT+CIPSERVER=1," + PORT, 10000UL );
    if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
    break;
  }
  if( lastStepResult == STEP_AT_RUN_SUCCESS || lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT )
  {
    DoSetupWiFiStarted = false;
    dbg.print( F("Ard: DoSetupWiFi finished in ") );
    dbg.print( ( millis() - DoSetupWiFiTimer ) );
    dbg.println( F(" milliseconds.") );
    if( lastStepResult == STEP_AT_RUN_SUCCESS )
    {
      dbg.println( F("Ard: WiFi intialised successfully.") );
      wifiState = WIFI_STATE_OK;
    }
    else
    {
      dbg.println( F("Ard: wifi setup failure.") );
      wifiState = WIFI_STATE_ERROR; //WIFI_STATE_SETUP_FAILED;
      DoSetupWifiRetries = DoSetupWifiRetries + 1;
    }
    SetLedsWifi();
  }
}

const char delimit = '|';
void ReadEspSerial() 
{
  // unsigned long timeout = 0UL;
  String stepATTerm = stepATSuccessTerm + delimit + stepATErrorTerm;
  int len = stepATTerm.length();
  // dbg.print( "Ard: stepATTerm: [" + stepATTerm + "], delimit:" );
  // dbg.println( delimit );
  // do
  // {
    while( true ) //esp.available() )
    {
      char inChar = (char)esp.read(); // get the new byte
      if( inChar == -1 )
      {
        break;
      }
      espString += inChar; // add it to the espString:
      // dbg.println( "Ard: " + inChar );
      // dbg.println( "Ard: espString: " + espString );
      if( len > 1 ) //&& espString.length() >= len ) 
      {
        String s = stepATTerm;
        int ind = s.indexOf( delimit );
        while( ind > -1 )
        {
          String s1 = s.substring( 0, ind );
          // dbg.println( "s1: [" + s1 + "]" );
          if( espString.indexOf( s1 ) > -1 )
          {         
            // dbg.println( "Ard: " + s1 + " found!" );
            if( stepATSuccessTerm.indexOf( s1 ) > -1 )
            {
              stepATTermSearch = STEP_AT_SUCCESS_TERM_FOUND;
            }
            else
            {
              stepATTermSearch = STEP_AT_ERROR_TERM_FOUND;
            }
          }
          String s2 = s.substring( ind + 1 );
          // dbg.println( "s2: [" + s2 + "]" );
          s = s2;
          ind = s.indexOf( delimit );
          if( ind == -1 )
          {
            if( espString.indexOf( s2 ) > -1 )
            {         
              // dbg.println( "Ard: " + s2 + " found!" );
              if( stepATSuccessTerm.indexOf( s2 ) > -1 )
              {
                stepATTermSearch = STEP_AT_SUCCESS_TERM_FOUND;
              }
              else
              {
                stepATTermSearch = STEP_AT_ERROR_TERM_FOUND;
              }
            }
          }
        }
        // if( espString.indexOf( stepATTerm ) > -1 )
        // {         
        //   if( stepATSuccessTerm.indexOf( stepATTerm ) > -1 )
        //   {
        //     stepATTermSearch = STEP_AT_SUCCESS_TERM_FOUND;
        //   }
        //   else
        //   {
        //     stepATTermSearch = STEP_AT_ERROR_TERM_FOUND;
        //   }
        // }
      }      

      if( inChar == '\n' )
      {
        if( espString.indexOf( "+IPD," ) > -1 )
        {
          int const size = 100;
          char buffer[size];
          espString.toCharArray( buffer, size );
          int ch_id, packet_len;
          sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);
          if( packet_len > 0 )
          {
            espRequest.ch_id = String( ch_id );
            espRequest.text = espString;
            espRequest.status = ESP_REQUEST_NEW;
            espString = "";
            dbg.print( F("New esprequest: ") );
            dbg.println( espRequest.text );            
          }
        }
        else
        {
          dbg.println( "Esp: " + espString );
          espString = "";
        }
      }
    }
    if( len > 1 && espString.length() > 0 )
    {
      espString = "";
      if( stepATTermSearch == STEP_AT_SUCCESS_TERM_FOUND )
      {
        dbg.println( "Ard: " + stepATSuccessTerm + " found!" );
      }
      else if( stepATTermSearch == STEP_AT_ERROR_TERM_FOUND )
      {
        dbg.println( "Ard: " + stepATErrorTerm + " found!" );
      }
    }

  //   if( timeout == 0 )
  //   {
  //     timeout = millis() + 10;  //pkon: read any last remaining bytes
  //   }
  // } while (millis() < timeout);
}

String CreateHtmlState()
{
  String content="Relay ";
  if( relayOn )
  {
    content += "ON" ;
  }
  else
  {
    content += "OFF" ;
  }
  return content;
}

String CreateHtmlMode()
{
  String content = "Mode ";
  if( mode == MODE_PERSON )
  {
    content += "person";
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    content += "two-wave";
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    content += "one-wave";
  }
  else if ( mode == MODE_WIFI )
  {
    content += "wifi";
  }
  return content;
}

String CreateHtmlStatus()
{
  String content = CreateHtmlState();
  content += "<br/>\n";
  content += CreateHtmlMode();
  return content;
}

void EspSendMode()
{
  EspSend( CreateHtmlMode() );
}

void EspSendStatus()
{
  EspSend( CreateHtmlStatus() );
}

void EspSendState()
{  
  EspSend( CreateHtmlState() );
}

void EspSendName()
{
  EspSend( DeviceName );
}

bool espSendStarted = false;
unsigned long espSendTimer = 0UL;
void EspSend( String content )
{
  if( stepATRunStatus != STEP_AT_RUN_RUNNING )
  {
    dbg.print( F("Ard: will EspSend [") );
    dbg.print( content );
    dbg.println( F("]") );
    espResponse.status = ESP_SEND_SEND;
    espResponse.content = content;
    DoEspSend();
  }
  else
  {
    dbg.print( F("Ard: cannot EspSend [") );
    dbg.print( content );
    dbg.print( F("] because stepATRunStatus: ") );
    dbg.println( stepATRunStatus );
  }
}

void DoEspSend()
{
  // dbg.print( F("espResponse.status: ") );
  // dbg.println( espResponse.status );
  // dbg.print( F("wifiState: ") );
  // dbg.println( wifiState );
  if( espResponse.status == ESP_SEND_NO_SEND || wifiState != WIFI_STATE_OK )
  {
    return;
  }

  if( !espSendStarted )
  {    
    espResponse.header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
    espResponse.header += "Content-Length:";
    espResponse.header += (int)(espResponse.content.length());
    espResponse.header += "\r\n\r\n";
    // esp.print("AT+CIPSEND=");
    // esp.print(espRequest.ch_id);
    // esp.print(",");
    // esp.println(espResponse.header.length() + espResponse.espResponse.content.length());

    espSendStarted = true;
    espSendTimer = millis();
    stepAT = 0;
    dbg.println( F("Ard: started DoEspSend with content ") );
    dbg.println( F("***********") );
    dbg.println( espResponse.content );
    dbg.println( F("***********") );
    // return;
  }

  byte lastStepResult = -1;
  RunStep( 
    0, 
    "AT+CIPSEND=" + espRequest.ch_id + "," + String( espResponse.header.length() + espResponse.content.length() ),
    8000UL,
    "> |OK", "ERROR"
  );

  if( PreviousStepResult( 1 ) == STEP_AT_RUN_SUCCESS )
  {
    // dbg.println( F("Ard: DoEspSend got AT+CIPSEND --OK--. Will send ") );
    // dbg.println( F(">>>>>>>>>>>>>>") );
    // dbg.println( espResponse.header + espResponse.content );
    // dbg.println( F("<<<<<<<<<<<<<<") );
    lastStepResult = RunStep( 1, ( espResponse.header + espResponse.content ), 10000UL, "CLOSED|OK", "ERROR" );
  }
  else if( PreviousStepResult( 1 ) == STEP_AT_RUN_FAIL || PreviousStepResult( 1 ) == STEP_AT_RUN_TIMEDOUT )
  {
    lastStepResult = RunStep( 1, "AT+CIPCLOSE=" + espRequest.ch_id, 1000UL );  
  }

  //this is necessary in case there is a fail of step {unStep( 1, ( espResponse.header + espResponse.content ), 10000UL, "CLOSED|OK|ERROR" );}
  if( PreviousStepResult( 2 ) == STEP_AT_RUN_FAIL || PreviousStepResult( 2 ) == STEP_AT_RUN_TIMEDOUT )
  {
    lastStepResult = RunStep( 2, "AT+CIPCLOSE=" + espRequest.ch_id, 1000UL );   
  }

  if( lastStepResult == STEP_AT_RUN_SUCCESS || 
      lastStepResult == STEP_AT_RUN_FAIL || 
      lastStepResult == STEP_AT_RUN_TIMEDOUT 
    )
  {
    dbg.println( F("Ard: DoEspSend finished.") );
    espSendStarted = false;
    espResponse.status = ESP_SEND_NO_SEND;
  }
}


String GetLogDataContent( struct ALogData data )
{
  String content = "device: " + DeviceName;
  content += ", actor: ";
  switch( data.actor )
  {
    case LOG_ACTOR_WIFI:
      content += "wifi";
    break;
    case LOG_ACTOR_BUTTON: 
      content += "button";
    break;
    case LOG_ACTOR_ULTRASOUND: 
      content += "ultrasound";
    break;
    case LOG_ACTOR_SYSTEM: 
      content += "system";
    break;
  }
  content += ", target: ";
  switch( data.target )
  {
    case LOG_TARGET_RELAY: 
      content += "relay";
    break;
    case LOG_TARGET_MODE: 
      content += "mode";
    break;
    case LOG_TARGET_NAME: 
      content += "name";
    break;
    case LOG_TARGET_SYSTEM: 
      content += "system";
    break;
  }
  return content + ", time: " + String( data.time ) + ", value: " + data.value;  
}

void CopyLogData( struct ALogData src, struct ALogData& dest )
{
  dest.empty  = src.empty;
  dest.actor  = src.actor;
  dest.target = src.target;
  dest.time = src.time;
  dest.value  = src.value;
}

void AddLogData( struct ALogData& data )
{
  // will add to the first empty space
  // if no empty space, then will shift records one place to make room
  bool emptyPlaceInd = -1;
  for( int emptyPlaceInd = 0 ; emptyPlaceInd < LOG_DATA_ARRAY_SIZE ; emptyPlaceInd++ )
  {
    if( logData[emptyPlaceInd].empty == 1 )
    {
      dbg.print( F("Ard: AddLogData copying {") );
      dbg.print( GetLogDataContent( data ) );
      dbg.print( F("} to position [") );
      dbg.print( emptyPlaceInd );
      dbg.print( F("]: {") );
      dbg.print( GetLogDataContent( logData[emptyPlaceInd] ) );
      dbg.println( F("}") );
      CopyLogData( data, logData[emptyPlaceInd] );
      // logData[emptyPlaceInd].empty  = data.empty;
      // logData[emptyPlaceInd].actor  = data.actor;
      // logData[emptyPlaceInd].target = data.target;
      // logData[emptyPlaceInd].time = data.time;
      // logData[emptyPlaceInd].value  = data.value;
      dbg.print( F("Ard: now [") );
      dbg.print( emptyPlaceInd );
      dbg.print( F("]= ") );
      dbg.print( GetLogDataContent( logData[emptyPlaceInd] ) );
      dbg.println( F("}.") );
      break;
    }
  }
  if( emptyPlaceInd == -1 )
  {
    for( int i = LOG_DATA_ARRAY_SIZE-1 ; i > 0; i-- )
    {
      CopyLogData( logData[i-1], logData[i] );
    }
    CopyLogData( data, logData[0] );
    dbg.print( F("Ard: AddLogData shifted old data and copied {") );
    dbg.print( GetLogDataContent( logData[emptyPlaceInd] ) );
    dbg.print( F("} to position [0]") );
  }
}

void Log( byte actor, byte target, String value )
{  
  dbg.println( F("Ard: will Log.") );
  ALogData data = { 0, actor, target, millis(), value };
  AddLogData( data );  
}


bool doLogStarted = false;
unsigned long DoLogTimer = 0UL;
unsigned long DoLogLastTime = 0UL;
void DoLog()
{
  // DoLog runs only when no HTTP Server communication is occuring
  if( ( wifiState == WIFI_STATE_SETTING_UP || wifiState == WIFI_STATE_OK ) && stepATRunStatus == STEP_AT_RUN_RUNNING )
  {
    // dbg.println( F("Ard: DoLog will NOT run!") );
    return;
  }
  bool nonEmptyLogRecordFound = false;
  for( int i = 0 ; i < LOG_DATA_ARRAY_SIZE ; i++ )
  {
    if( logData[i].empty != 1 )
    {
      nonEmptyLogRecordFound = true;
      // dbg.print( F("Ard: Found non-empty log at position [") );
      // dbg.print( i );
      // dbg.println( F("].") );
      break;
    }
  }
  if( !nonEmptyLogRecordFound && 
      wifiState != WIFI_STATE_LOG_OK && 
      wifiState != WIFI_STATE_LOG_TRANSMITTING && 
      wifiState != WIFI_STATE_LOG_RESETTING_SERVER_MODE )
  {
    return;
  }

  if( !doLogStarted )
  {    
    unsigned long time = millis();
    if( time - DoLogLastTime < LOG_MIN_TIMESPAN )
    {
      // dbg.print( F("Ard: Will not DoLog because [millis() - DoLogLastTime < LOG_MIN_TIMESPAN] (") );
      // dbg.println( String(time) + " - " + String(DoLogLastTime) + " < " + String( LOG_MIN_TIMESPAN ) + ")" );
      return;
    }
    //ESPReset();
    doLogStarted = true;
    DoLogTimer = millis();
    stepAT = 0;
    wifiState = WIFI_STATE_LOG_SETTING_UP;
    dbg.println( F("Ard: started DoLog.") );
    // return;
  }

  bool wifiServerModeRestored = false;
  byte lastStepResult = -1;
  while( true )
  {
    // setup: set mode to tcp client
    if( wifiState != WIFI_STATE_LOG_OK || wifiState == WIFI_STATE_LOG_SETTING_UP )
    {      
      // dbg.println( F("Ard: setting up log wifi.") );
      lastStepResult = RunStep( 0, "AT+CIPSERVER=0", 10000UL ); // not server anymore
      if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
      lastStepResult = RunStep( 1, "AT+CIPMUX=0", 10000UL ); // no multiple connections
      if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
      lastStepResult = RunStep( 2, "AT+CIPSTART=\"TCP\",\"" + String( LOG_SERVER_IP_ADDRESS ) + "\"," + String(LOG_SERVER_TCP_PORT), 10000UL );
      if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
      if( PreviousStepResult( 3 ) == STEP_AT_RUN_SUCCESS )
      {
        dbg.println( F("Ard: log wifi setup successfull.") );
        wifiState = WIFI_STATE_LOG_OK;
      }
    }

    // send data
    if( wifiState == WIFI_STATE_LOG_OK || wifiState == WIFI_STATE_LOG_TRANSMITTING )
    {
      // dbg.print( F("wifiState = ") );
      // dbg.print( wifiState );
      // dbg.println( F(" i.e. == WIFI_STATE_LOG_OK") );
      bool allLogsEmpty = true;      
      for( int i = LOG_DATA_ARRAY_SIZE-1 ; i >= 0 ; i-- )
      {
        if( logData[i].empty != 1 )
        {
          wifiState = WIFI_STATE_LOG_TRANSMITTING;
          allLogsEmpty = false;
          String content = GetLogDataContent( logData[i] );
          dbg.print( F("Ard: will transmit non-empty log at position [") );
          dbg.print( i );
          dbg.print( F("]: {") );
          dbg.print( content );
          dbg.println( F("}.") );

          lastStepResult = RunStep( 3, "AT+CIPSEND=" + String( content.length() ), 8000UL, "> |SEND OK", "ERROR" );

          if( PreviousStepResult( 4 ) == STEP_AT_RUN_SUCCESS )
          {
            lastStepResult = RunStep( 4, content, 8000UL );
          }

          if( PreviousStepResult(5) == STEP_AT_RUN_SUCCESS )
          {
            dbg.println( F("Ard: {RunStep( 4, content...} succeeded.") );
            dbg.print( F("Ard: transmission was successfull and lastStepResult == STEP_AT_RUN_SUCCESS. Setting logdata[") );
            dbg.print( i );
            dbg.println( F("].empty=1") );
            logData[i].empty = 1;
            stepAT = 3; //in order to be able to restart the for loop and start trasnmission of the next non-empty log record       
          }
          
          break; // every time we find a non-empty log we break so that allLogsEmpty remains false
        }
      }

      dbg.print( F("allLogsEmpty: " ) ); dbg.println( allLogsEmpty );
      // no more logs to send or cannot send due to errors
      if( allLogsEmpty || lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT )
      {
        wifiState = WIFI_STATE_LOG_RESETTING_SERVER_MODE;
        stepAT = 6; //in order to continue with RunStep( 6, "AT+CIPCLOSE"
        dbg.println( F("Ard: no more non-empty log records to transmit, or an error occurred." ) );
      }
    }
    if( wifiState == WIFI_STATE_LOG_RESETTING_SERVER_MODE )
    {
      lastStepResult = RunStep( 6, "AT+CIPCLOSE", 1000UL ); // close TCP connection
      // set mode back to tcp server
      lastStepResult = RunStep( 7, "AT+CIPMUX=1", 1000UL );
      if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
      lastStepResult = RunStep( 8, "AT+CIPSERVER=1," + String(PORT), 1000UL );
      if( lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT ) break;
      if( PreviousStepResult(9) == STEP_AT_RUN_SUCCESS )
      {
        dbg.println( F("Ard: restored wifi to server mode.") );
        wifiServerModeRestored = true;
      }
    }
    break;
  }

  if( wifiServerModeRestored  || 
      lastStepResult == STEP_AT_RUN_FAIL || 
      lastStepResult == STEP_AT_RUN_TIMEDOUT 
    )
  {
    dbg.print( F("Ard: DoLog finished in ") );
    dbg.print( ( millis() - DoLogTimer ) );
    dbg.println( F(" milliseconds." ) );
    if( !wifiServerModeRestored )
    {
      wifiState = WIFI_STATE_NOT_SETUP; // this will re-setup the rest of the wifi server mode
    }
    else
    {
      wifiState = WIFI_STATE_OK;
    }
    doLogStarted = false;
    DoLogLastTime = millis();
  }
}