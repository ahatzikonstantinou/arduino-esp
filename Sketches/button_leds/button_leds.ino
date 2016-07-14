#include "EEPROM.h"
const int EEsize = 1024; // size in bytes of arduino UNO EEPROM
const int ModeAddress = 0;  //where the ledState is saved

const byte WiFiLed = 6;
const byte PersonLed = 7;
const byte TwoWaveLed = 8;
const byte OneWaveLed = 9;

const byte BUTTON = 2;  // Button connected to digital pin 10
const byte relayPin = 5;
bool relayOn = false;

#define MODE_PERSON 0  //0: switch on when detect person
#define MODE_TWO_WAVE 1    //1: switch on when detect two gesture wave
#define MODE_ONE_WAVE 2    //2: switch on when detect single gesture wave
#define MODE_WIFI 3    //3: controlled by wifi only

const byte maxMode = MODE_WIFI;
volatile int mode = 0;  

#define MIN_BUTTON_REPRESS 50 // x msecs acceptable between 2 button presses
#define DEBOUNCE_DELAY 10 // x msecs to guard against button debounce
volatile int lastCallTime = 0;
volatile int lastButtonState = 0;

#define dbg Serial

void setup() 
{
  dbg.begin(115200);
  dbg.println("begin.");

  pinMode(WiFiLed,OUTPUT);
  pinMode(PersonLed,OUTPUT);
  pinMode(TwoWaveLed,OUTPUT);
  pinMode(OneWaveLed,OUTPUT);
  pinMode(relayPin,OUTPUT);
  
  // the button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON, INPUT_PULLUP ); 

  // only one interrupt attached per pin
  //attachInterrupt (0, pinChange, CHANGE);  // attach interrupt handler
  EnableButton();
  
}

void loop() 
{
  delay( 10 );  
}

void TurnLedsOff()
{
	digitalWrite(WiFiLed, LOW);
	digitalWrite(PersonLed, LOW);
	digitalWrite(TwoWaveLed, LOW);
	digitalWrite(OneWaveLed, LOW);
}

void SetLeds()
{
	TurnLedsOff();
  if( mode == MODE_PERSON )
  {
    digitalWrite(PersonLed, HIGH);   // sets the LED on
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    digitalWrite(TwoWaveLed, HIGH);    // sets the LED on
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    digitalWrite(OneWaveLed, HIGH);    // sets the LED on
  }
  else if ( mode == MODE_WIFI )
  {
    digitalWrite(WiFiLed, HIGH);   // sets the LED on
  }
}

// IMPORTANT NOTE: Do NOT put Serial.println in interrupt functions!!! Or else nothing works
void EnableButton()
{
	attachInterrupt (0, pinChange, CHANGE);  // attach interrupt handler
}
void DisableButton()
{
	detachInterrupt (0);  // detach interrupt handler
}

// Interrupt Service Routine (ISR)
void pinChange()
{
	//DisableButton();
  int time = millis();
  if( time - lastCallTime < MIN_BUTTON_REPRESS )
  {
		//EnableButton();
		return;
	}
	lastCallTime = time;
  int buttonState = digitalRead(BUTTON);
  
  delay( DEBOUNCE_DELAY );

  // alternative mode: every time the button is depressed (HIGH) toggle the led
  if( buttonState == HIGH && digitalRead(BUTTON) == HIGH )
  {
      GoToNextMode();
  }
  //EnableButton();
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
    dbg.println( "mode person" );
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    dbg.println( "mode two-wave" );
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    dbg.println( "mode one-wave" );
  }
  else if ( mode == MODE_WIFI )
  {
    dbg.println( "mode wifi" );
  }
}

void ChangeMode( int newMode )
{
  if( mode < 0 || mode > maxMode )
  {
    dbg.print( "Error. Cannot change mode to " );
    dbg.print( newMode );
    dbg.print( ". Mode must be a number between 0 and " );
    dbg.print( maxMode );
    dbg.print( ". Mode will not be changed!" );
  }
  
  mode = newMode ;
  dbgMode();
  EEPROM.write( ModeAddress, mode ); // save to eprom
  
  SetLeds();
}
