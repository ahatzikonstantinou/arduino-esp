#include <Arduino.h>
#include "ultrasound.h"
#include "detect.h"

unsigned int ULTRA_DIST_PERSON = 100;  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
unsigned int ULTRA_DIST_WAVE = 30;  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.
unsigned long PERSON_PRESENT_DELAY = 2000UL; // recheck after x millisenonds to ensure a person is still in front of the ultrasound sensor
unsigned long PERSON_ABSENT_DELAY = 4000UL; // recheck after x millisenonds to ensure a person is still away from the ultrasound sensor
unsigned long TWO_WAVE_DURATION = 500UL;  // the max duration of a two wave gesture
unsigned long TWO_WAVE_DELAY = 300UL;  // the delay betweeb two waves in a two wave gesture
unsigned long ONE_WAVE_DELAY = 300UL;  // the delay until the user moves the hand away for a one wave gesture

unsigned int GetUltraDistPerson()
{
  return ULTRA_DIST_PERSON;
}

void SetUltraDistPerson( unsigned int d )
{
  ULTRA_DIST_PERSON = d;
}

unsigned int GetUltraDistWave()
{
  return ULTRA_DIST_WAVE;
}

void SetUltraDistWave( unsigned int d )
{
  ULTRA_DIST_WAVE = d;
}

unsigned long GetPersonPresentDelay()
{
  return PERSON_PRESENT_DELAY;
}

void SetPersonPresentDelay( unsigned long d )
{
  PERSON_PRESENT_DELAY = d;
}

unsigned long GetPersonAbsentDelay()
{
  return PERSON_ABSENT_DELAY;
}

void SetPersonAbsentDelay( unsigned long d )
{
  PERSON_ABSENT_DELAY = d;
}

unsigned long GetTwoWaveDuration()
{
  return TWO_WAVE_DURATION;
}

void SetTwoWaveDuration( unsigned long d )
{
  TWO_WAVE_DURATION = d;
}

unsigned long GetTwoWaveDelay()
{
  return TWO_WAVE_DELAY;
}

void SetTwoWaveDelay( unsigned long d )
{
  TWO_WAVE_DELAY = d;
}

unsigned long GetOneWayDelay()
{
  return ONE_WAVE_DELAY;
}

void SetOneWayDelay( unsigned long d )
{
  ONE_WAVE_DELAY = d;
}


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
