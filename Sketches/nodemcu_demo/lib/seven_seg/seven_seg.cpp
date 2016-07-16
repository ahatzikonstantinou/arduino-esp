#include "seven_seg.h"
#include "globals.h"
#include "my_wifi.h"

volatile byte wifiBlinkMode = BLINK_WIFI_NONE;
volatile bool wifiBlinkOn = false;

void InitLeds()
{
  pinMode( a7Seg, OUTPUT );
  pinMode( b7Seg, OUTPUT );
  pinMode( c7Seg, OUTPUT );
  pinMode( d7Seg, OUTPUT );
  pinMode( e7Seg, OUTPUT );
  pinMode( f7Seg, OUTPUT );
  pinMode( g7Seg, OUTPUT );
  pinMode( dp7Seg, OUTPUT );
  TurnLedsOff();
}

void BlinkWifiLed()
{
  // if( blinkMode = BLINK_WIFI_NONE )
  // {
  //   return;
  // }

  noInterrupts();
  timer1_isr_init();

  // unsigned long t = millis();
  if( wifiBlinkOn )
  {
    // if( t - BlinkWifiTimer < WIFI_FAIL_BLINK_TIME_OFF )
    // {
    //   return;
    // }
  //  Timer1.initialize( WIFI_FAIL_BLINK_TIME_OFF );
    timer1_write( ESP.getCycleCount() + WIFI_FAIL_BLINK_TIME_OFF );
  }
  else
  {
    // if( t - BlinkWifiTimer < WIFI_FAIL_BLINK_TIME_ON )
    // {
    //   return;
    // }
  //  Timer1.initialize( WIFI_FAIL_BLINK_TIME_ON );
    timer1_write( ESP.getCycleCount() + WIFI_FAIL_BLINK_TIME_ON );
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
  // Timer1.attachInterrupt( BlinkWifiLed );
  timer1_attachInterrupt( BlinkWifiLed );
  interrupts();
}

void BlinkWifiLedStart( byte blinkMode )
{
  wifiBlinkMode = blinkMode;
  BlinkWifiLed();
}

void BlinkWifiLedStop()
{
  // blinkMode = BLINK_WIFI_NONE;
  noInterrupts();
  // Timer1.detachInterrupt();
  timer1_detachInterrupt();
  interrupts();
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
  switch( GetWifiState() )
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
  if( GetMode() == MODE_PERSON )
  {
    SetLedsPerson();
  }
  else if ( GetMode() == MODE_TWO_WAVE )
  {
    SetLedsTwoWave();
  }
  else if ( GetMode() == MODE_ONE_WAVE )
  {
    SetLedsOneWave();
  }

  //WIFI is always on (depending on its state)
  SetLedsWifi();
}
