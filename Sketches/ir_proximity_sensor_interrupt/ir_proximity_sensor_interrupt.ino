
#define IRPin 3

void setup() 
{
  pinMode( IRPin, INPUT_PULLUP );
  attachInterrupt (1, IRPinChange, CHANGE);
}

void loop() 
{
}

// Interrupt Service Routine (ISR)
void IRPinChange()
{
  // when an obstacle is detected the pin goes to LOW
  if( digitalRead( IRPin ) == LOW )
  {
    
  }
}
