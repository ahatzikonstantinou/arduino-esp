#include <Arduino.h>
#include "ultrasound.h"
#include "detect.h"


unsigned long PersonPresentTimer = 0UL;
bool PersonPresentStarted = false;
// This function will wait for a while in case person moves quickly in front of the ultrasound sensor
byte DetectPersonPresent()
{
  if( !PersonPresentStarted )
  {
    PersonPresentTimer = millis();
    PersonPresentStarted = true;

  	float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

		float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

		float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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
    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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
    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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

    float distance = SampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO );
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
