#define dbg Serial

#define OPTO 5 // analog 5
const int optoPin = 3;
void setup() 
{
  dbg.begin(9600);
//  pinMode( optoPin, INPUT );
}

void loop() 
{
  String msg = "Optocoupler ";
  int optoState = digitalRead( optoPin );
  if( optoState == HIGH )
  {
    msg += "ON";
  }
  else
  {
    msg += "OFF";
  }

//  int x = analogRead( OPTO );
//  msg += x;
  dbg.println( msg );
//  delay( 1000 );
}
