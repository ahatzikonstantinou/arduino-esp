#include <TimerOne.h> // Header file for TimerOne library
#define UltraSamplePeriod 500000 // microseconds
#define UltraSampleNum 10  //how many samples to take every UltraSamplePeriod
#define UltraSampleInterval 10000  // microseconds to wait between each ultra sample
int ultraSamples[UltraSampleNum];
#define ULTRAECHO 11
#define ULTRATRIG 12
#define ULTRA_DIST_THRESHHOLD 20  //in centimeters. Beyond that relay is switched off. Less than this, relai is swtiched on.

#define relayPin 9

#define dbg Serial

void setup() 
{
  dbg.begin(9600);
  
  // setup ultrasound sensor
  pinMode( ULTRATRIG, OUTPUT );
  pinMode( ULTRAECHO, INPUT );
  
  // setup timer  
//  Timer1.initialize(UltraSamplePeriod);
//  Timer1.attachInterrupt( SampleUltra );

  SetupRelay();
}

void loop() 
{
  SampleUltra();
}


void SetupRelay() 
{
  pinMode(relayPin,OUTPUT);
//  SwitchRelay( false );
  digitalWrite(relayPin, LOW);
  
}

void SwitchRelay( boolean on )
{
  if( on )
  {
    digitalWrite(relayPin, HIGH);
//    SaveRelayState( HIGH );
  }
  else
  {
    digitalWrite(relayPin, LOW);
//    SaveRelayState( LOW );
  }
}

int GetUltraDistance()
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
    
    return distance;
}

void SampleUltra()
{
  dbg.println( "Timer expired" );
//  noInterrupts(); // disable interrupts while sampling the ultrasound sensor
//  interrupts();  //reenable interupts
  
  long avgDistance = 0;  // centimeters
  long totalDistance = 0;
  long validSampleCount = 0;
  for( int i = 0 ; i < UltraSampleNum ; i++ )
  {
    long distance = GetUltraDistance();
    if( distance < 200 && distance > 0 )
    {
//      ultraSamples[i] = distance;
      avgDistance += distance;
      validSampleCount++;
    }
    delayMicroseconds( UltraSampleInterval );
  }
//  dbg.print( "Total distance: " );
//  dbg.println( totalDistance );
  avgDistance = avgDistance / validSampleCount;
  dbg.print( "Avg distance: " );
  dbg.print( avgDistance );
  if( avgDistance >= 200 || avgDistance <= 0 )
  {
    dbg.println(" cm. Out of range");    
  }
  else 
  {
    dbg.println(" cm");
  }

  if( avgDistance > 0 && avgDistance < ULTRA_DIST_THRESHHOLD ) 
  {  
    SwitchRelay( true );
  }
  else 
  {
    SwitchRelay( false );
  }
  
//  interrupts();  //reenable interupts
}
