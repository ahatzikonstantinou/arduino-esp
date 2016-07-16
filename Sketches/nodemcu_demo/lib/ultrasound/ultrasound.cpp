#include "ultrasound.h"

float SnglSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin )
{
    unsigned long duration = 0;
    float distance = -1;
		while( distance < MinDistance || distance > MaxDistance )
		{
			// Serial.println( "Getting distance" );
      delay(100);  // wait for measurement to finish (max 400 cm * 58 uSec = 23uSec)
      digitalWrite(TriggerPin, LOW);  // Added this line
      delayMicroseconds(2); // Added this line
      digitalWrite(TriggerPin, HIGH);
      //  delayMicroseconds(1000); - Removed this line
      delayMicroseconds(10); // Added this line
      digitalWrite(TriggerPin, LOW);
      duration = pulseIn(EchoPin, HIGH, 35000);
			// Serial.print( "Duration:" );
			// Serial.println( duration );
      distance = ( (float) duration ) / 58;  // centimeters
    }
    return distance;
}


float CirclSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin )
{
  const int maxTries = 30;
  const int minOutliers = 0;
  const int sampleCount = 5;
  float samples[sampleCount] ;
  float sum = 0.0;
  for( int i = 0 ; i < sampleCount ; i++ )
  {
    // samples[i] = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
    samples[i] = IntSampleUltra( TriggerPin, EchoPin );
    if( -1 == samples[i] )
    {
      samples[i] = 0;
    }
    //Serial.print( String( samples[i] ) + ", " );
    sum += samples[i];
  }
  float avg = sum/sampleCount;
  // Serial.println( "**************\nsum: " + String( sum ) + ", avg: " + String( avg ) );
  // Serial.println( "Initial distance: " + String( avg ) + " (sum: " + String(sum) + ")" );

  int outliers = 0;
  float const outlierMaxThreshhold = 1.4; // the threshold is actually 40%
  float const outlierMinThreshhold = 0.6;
  float maxThreshold = outlierMaxThreshhold*avg;
  float minThreshold = outlierMinThreshhold*avg;
  for( int i = 0 ; i < sampleCount ; i++ )
  {
    if( samples[i] > maxThreshold || samples[i] < minThreshold )
    {
      // sum -= samples[i];
      outliers ++;
    }
  }

  int tries = 0;
  int j = 0;
  while( outliers > minOutliers && tries < maxTries )
  {
    // if( j == 0 )
    // {
    //   Serial.println( "---------------------" );
    // }
    // replace a sample and recalculate sum
    // Serial.print( "Replacing sample[" + String( j ) + "] = " + String( samples[j] ) + " with " );
    sum -= samples[j];
    // samples[j] = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
    samples[j] = IntSampleUltra( TriggerPin, EchoPin );
    if( -1 == samples[j] )
    {
      samples[j] = 0;
    }
    sum += samples[j];
    // Serial.print( String( j ) + ": " + String( samples[j] ) );

    // recalculate avg
    avg = sum/sampleCount;

    // recalculate outliers
    maxThreshold = outlierMaxThreshhold*avg;
    minThreshold = outlierMinThreshhold*avg;
    outliers = 0;
    for( int i = 0 ; i < sampleCount ; i++ )
    {
      if( samples[i] > maxThreshold || samples[i] < minThreshold )
      {
        // Serial.println( "***" + String( samples[i] ) + "*** (sum: " + String(sum) + ", maxThreshold:" + String(maxThreshold) +", minThreshold:" + String(minThreshold) + ")" );
        // sum -= samples[i];
        outliers ++;
      }
    }
    // Serial.println( ", outliers: " + String( outliers ) );
    j = (j+1)%sampleCount;
    tries++;
  }
  // Serial.println( "Total " + String( tries ) + " retries for distance." );

  if( tries == maxTries )
  {
    // Serial.println( "MaxTries reached, aborting..." );
    avg = -1;
  }
  else if( outliers > 0 )
  {
    avg = sum/(sampleCount - outliers);
  }

  // Serial.println( "Distance: " + String( avg ) + ", outliers: " + String( outliers ) );
  // Serial.println( "Distance: " + String(avg) + " (retries: " + String( tries ) + ")" );
  return avg;
}

volatile unsigned long UltraTimer = 0;
volatile unsigned long UltraState = 0;

float IntSampleUltra( byte TriggerPin, byte EchoPin )
{
  float value = -1;
  UltraTimer = 0;
  UltraState = 0;
  noInterrupts();
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  interrupts();

  delay(25);  // wait for measurement to finish (max 400 cm * 58 uSec = 23uSec)

  if (UltraState == 2)
  {
    value = (float)UltraTimer / 58;
  }
  // Serial.println( "Distance: " + String( value ) );
  return value;
}

void UltraInterrupt()
{
  byte pinState = digitalRead(ULTRAECHO);
  if (pinState == 1) // Start of pulse
  {
    UltraState = 1;
    UltraTimer = micros();
  }
  else // End of pulse, calculate timelapse between start & end
  {
    UltraState = 2;
    UltraTimer = micros() - UltraTimer;
  }
}

float MultiSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin )
{
  const int sampleCount = 15;
  float samples[sampleCount] ;
  float sum = 0;
  for( int i = 0 ; i < sampleCount ; i++ )
  {
    // samples[i] = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
    samples[i] = IntSampleUltra( TriggerPin, EchoPin );
    //Serial.print( String( samples[i] ) + ", " );
    sum += samples[i];
  }
  float avg = sum/sampleCount;
  Serial.println( "sum: " + String( sum ) + ", avg: " + String( avg ) );

  int outliers = 0;
  float const maxThreshold = 1.4*avg;  // the threshold is actually 10%
  float const minThreshold = 0.6*avg;
  for( int i = 0 ; i < sampleCount ; i++ )
  {
    if( samples[i] > maxThreshold || samples[i] < minThreshold )
    {
      sum -= samples[i];
      outliers ++;
    }
  }
  if( outliers > (sampleCount/2) )
  {
    avg = -1;
  }
  else if( outliers > 0 )
  {
    avg = sum/(sampleCount - outliers);
  }

  Serial.println( "Distance: " + String( avg ) + ", outliers: " + String( outliers ) );
  return avg;
}

  /*
   * Very simple guard against odd readings. If two consecutive readings differ by more than 10% read again.
   */
float DblSampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin )
{
    float d1 = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
    float d2 = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );

    long diff = abs( d2 -d1 );
    if( diff > 0.2*d1 )
    {
      Serial.println( "d1: " + String( d1 ) + ", d2: " + String(d2) + " trying again.");
      return DblSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
    }

    Serial.println( "Distance: " + String( d1 ) );
    return d1;
}

void InitUltra()
{
  pinMode( ULTRATRIG, OUTPUT );
  pinMode( ULTRAECHO, INPUT_PULLUP );	// IMPORTANT: HC-SR04 does not reaspond if pinMode for echo is just INPUT

  attachInterrupt( ULTRAECHO, UltraInterrupt, CHANGE );
}

float SampleUltra( int MaxDistance, int MinDistance, byte TriggerPin, byte EchoPin )
{
  //change to whatever suits you best
  // float distance = MultiSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
  // float distance = DblSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
  // float distance = SnglSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
  // return IntSampleUltra( TriggerPin, EchoPin );
  float distance = CirclSampleUltra( MaxDistance, MinDistance, TriggerPin, EchoPin );
  // Serial.println( "Distance: " + String( distance ) );
  return distance;
}
