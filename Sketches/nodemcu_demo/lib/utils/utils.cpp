#include "utils.h"
#include <globals.h>
#include <seven_seg.h>

volatile byte mode = 0;
char DeviceName[DEVICE_NAME_SIZE] = "cupboard-multi-switch";
bool relayOn = false; // true when the relay is on

byte GetMode()
{
  return mode;
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
    Serial.println( "mode person" );
  }
  else if ( mode == MODE_TWO_WAVE )
  {
    Serial.println( "mode two-wave" );
  }
  else if ( mode == MODE_ONE_WAVE )
  {
    Serial.println( "mode one-wave" );
  }
  else if ( mode == MODE_WIFI )
  {
    Serial.println( "mode wifi" );
  }
}

void ChangeMode( int newMode )
{
  if( mode < 0 || mode > maxMode )
  {
    //Serial.print( "Error. Cannot change mode to " );
    //Serial.print( newMode );
    //Serial.print( ". Mode must be a number between 0 and " );
    //Serial.print( maxMode );
    //Serial.print( ". Mode will not be changed!" );
    return;
  }

  Serial.println( "mode was " + String( mode ) + ", will become " + String( newMode ));
  mode = (byte) newMode ;
  dbgMode();

  //TODO: fix for esp EEPROM.put( ModeAddress, mode ); // save to eprom

  SetLeds();
}

char* GetDeviceName()
{
  return DeviceName;
}

void UpdateDeviceName( String name )
{
  if( name.length() > 0 )
  {
    name.toCharArray( DeviceName, DEVICE_NAME_SIZE );
  }
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
  digitalWrite(relayPin, on ? LOW : HIGH );
  //dbg.print( "Turned relay " );
  //on ? dbg.println( "ON" ) : dbg.println( "OFF" );
}

bool GetRelayOn()
{
  return relayOn;
}
