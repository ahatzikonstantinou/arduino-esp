#include "EEPROM.h"

const int EEsize = 1024; // size in bytes of arduino UNO EEPROM
const int LEDStateAddress = 0;  //where the ledState is saved

const byte BUTTON = 2;
const byte LED = 13;
volatile int ledState = LOW;
const int debounceDelay = 50; // 100 msecs to guard against button debounce
volatile int lastCallTime = 0;
volatile int lastButtonState = 0;

#define dbg Serial

void setup() {
  pinMode (LED, OUTPUT);  // so we can update the LED
//  digitalWrite( LED, ledState );  // start with the led switched off
  digitalWrite( LED, GetLedState() ); // set the LED according to what is saved in EEPROM
  
  // our button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON, INPUT_PULLUP ); 

// old way to declare input pin and pullup resistor  
//  pinMode( BUTTON, INPUT );
//  digitalWrite( BUTTON, HIGH );
  
  // only one interrupt attached per pin
  attachInterrupt (0, pinChange, CHANGE);  // attach interrupt handler
//  attachInterrupt (0, pinRISING, RISING);  // attach interrupt handler
//  attachInterrupt (0, pinFALLING, FALLING);  // attach interrupt handler

  dbg.begin(9600);
  dbg.println("begin.");
}

void loop() {
}

void pinRISING()
{
//  digitalWrite (LED, HIGH);
  dbg.println( "BUTTON RISING" );
  if( !Bounce() )
  {
//    ledState = ledState^1;
    ToggleLed();
  }
}

void pinFALLING()
{
  digitalWrite (LED, LOW);
  dbg.println( "BUTTON FALLING" );
}

boolean Bounce()
{
  // The interrupt may be called multiple times on the same button press (or release)
  // So check only for real change
  int time = millis();
  int timelapse = time - lastCallTime;
//  dbg.print( "Time (millisecs) since last call: ");
//  dbg.println( timelapse );
  lastCallTime = time;
  if( timelapse < debounceDelay )
  {
//    dbg.println( "Bounce detected, exiting..." );
    return 1;
  }
  dbg.print( "Time (millisecs) since last call: ");
  dbg.println( timelapse );
  return 0;
}

// Interrupt Service Routine (ISR)
void pinChange()
{
  if( Bounce() )
  {
    return ;
  }
  
  int buttonState = digitalRead (BUTTON);
  dbg.print( "BUTTON state: ");
  dbg.println( buttonState );
//  if( lastButtonState == buttonState )
//  {
//    return;
//  }
//  
//  lastButtonState = buttonState;
//  dbg.print( "BUTTON: ");
//  dbg.println( digitalRead (BUTTON) );
  
//  ledState = ledState^1;
//  dbg.print( "LED switch to " ); dbg.println( ledState );
//  digitalWrite( LED, ledState );

//  if( buttonState == LOW)  // button pressed
//  {
//    digitalWrite (LED, HIGH);  // turn led on
//    dbg.println( "BUTTON LOW => LED HIGH" );
//  }
//  else  // button not pressed
//  {
//    digitalWrite (LED, LOW); // turned led off
//    dbg.println( "BUTTON HIGH => LED LOW" );
//  }

  // alternative mode: every time the button is depressed (HIGH) toggle the led
  if( buttonState == HIGH )
  {
      ToggleLed();
  }
}

void ToggleLed()
{
  int ledState = digitalRead( LED );
  if( ledState == HIGH )
  {
    digitalWrite( LED, LOW );
    SaveLedState( LOW );
  }
  else
  {
    digitalWrite( LED, HIGH );    
    SaveLedState( HIGH );
  }
}

// HIGH and LOW are integer constants see http://elabz.com/arduino-code-tidbits-1-declaring-an-array-with-pin-values/
void SaveLedState( int state )
{
  const byte high = 1;
  const byte low = 2;
  if( HIGH == state )
  {
    EEPROM.write( LEDStateAddress, high );
  }
  else
  {
    EEPROM.write( LEDStateAddress, low );
  }
}

int GetLedState()
{
  byte state = EEPROM.read( LEDStateAddress );
  if( 1 == state )
  {
    return HIGH;
  }
  else
  {
    return LOW;
  }
}
