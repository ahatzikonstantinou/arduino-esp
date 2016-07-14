#include "button.h"
#include <utils.h>
#include <Arduino.h>

volatile int lastButtonCallTime = millis();
volatile int lastButtonState = 1; // if this is LOW(0) then at boot the change button state is triggered and mode advances to the next

unsigned long ButtonDebounceTimer = 0UL;
bool DebounceButton = false;
void ReadButton()
{
  int buttonState = digitalRead( BUTTON_PIN );
  if( !DebounceButton )
  {
    unsigned long elapsedTime = millis() - lastButtonCallTime;
    if( elapsedTime > BUTTON_MIN_REPRESS )
    {
      if( buttonState == HIGH && buttonState != lastButtonState )
      {
        Serial.print( "Checking button after ");
        Serial.print( elapsedTime );
        Serial.println( " milliseconds" );
        Serial.print( "elapsedTime is valid ( > ");
        Serial.print( BUTTON_MIN_REPRESS );
        Serial.println( " )");
        Serial.println( "button was low and now is high. Will debounce." );
        lastButtonCallTime = millis();
        DebounceButton = true;
      }

      lastButtonState = buttonState;
    }
  }
  else
  {
    //wait until we are past the DEBOUNCE_DELAY
    if( millis() - lastButtonCallTime > BUTTON_DEBOUNCE_DELAY )
    {
      // //Serial.println( "Debouncing button..." );
      DebounceButton = false;
      int highCount = 0;
      //get a few samples to avoid false positives or false negatives
      for( int i = 0 ; i < BUTTON_SAMPLE_NUM ; i++ )
      {
        if( digitalRead(BUTTON_PIN) == HIGH )
        {
          highCount++;
        }
      }

      // every time the button is depressed (HIGH) toggle the led
      if( highCount > ( BUTTON_SAMPLE_NUM/2 ) )
      {
        Serial.println( "Button is high" );
        GoToNextMode();
      }
    }
  }
}
