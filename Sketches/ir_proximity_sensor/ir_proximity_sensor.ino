#define dbg Serial

//const int IRPin = 5; //analog A5
const int IRPin = 3; //digital 3

void setup() 
{
  dbg.begin(9600);
  pinMode( IRPin, INPUT );    
}

void loop() 
{
//  int x = analogRead( IRPin );
  int x = digitalRead( IRPin );
  dbg.print( "x = " );
  dbg.println( x );
  delay( 100 );
}
