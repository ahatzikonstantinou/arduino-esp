#include <TimerOne.h>                           // Header file for TimerOne library

const byte BUTTON = 2;
const byte LED = 13;
const int debounceDelay = 100; // 100 msecs to guard against button debounce
volatile int lastCallTime = 0;
volatile int lastButtonState = 0;

// When in working mode, the button works as usual. When in programming mode
// the button cycles between enabling and disabling the ultrasound
volatile int mode = 0; // 0:working, 1: programming

#define dbg Serial

void setup() {
  pinMode (LED, OUTPUT);  // so we can update the LED
  digitalWrite( LED, LOW );  // start with the led swotched off

  // our button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON, INPUT_PULLUP ); 

// old way to declare input pin and pullup resistor  
//  pinMode( BUTTON, INPUT );
//  digitalWrite( BUTTON, HIGH );
  
  // only one interrupt attached per pin
  attachInterrupt (0, pinChange, CHANGE);  // attach interrupt handler
//  attachInterrupt (0, pinRISING, RISING);  // attach interrupt handler
//  attachInterrupt (0, pinFALLING, FALLING);  // attach interrupt handler
    
  Timer1.initialize(1000000);
  Timer1.attachInterrupt( timerISR );
  Timer1.stop();    //stop the counter

  dbg.begin(9600);
  dbg.println("begin.");
}

void loop() {
}

void pinRISING()
{
  digitalWrite (LED, HIGH);
  dbg.println( "BUTTON RISING" );
}

void pinFALLING()
{
  digitalWrite (LED, LOW);
  dbg.println( "BUTTON FALLING" );
}

// Interrupt Service Routine (ISR)
void pinChange()
{
  // The interrupt may be called multiple times on the same button press (or release)
  // So check only for real change
  int time = millis();
  int timelapse = time - lastCallTime;
  dbg.print( "Time (millisecs) since last call: ");
  dbg.println( timelapse );
  lastCallTime = time;
  if( timelapse < debounceDelay )
  {
    dbg.println( "Bounce detected, exiting..." );
  }

  int buttonState = digitalRead (BUTTON);
  if( lastButtonState == buttonState )
  {
    return;
  }
  
  lastButtonState = buttonState;
  dbg.print( "BUTTON: ");
  dbg.println( digitalRead (BUTTON) );
  
  if( buttonState == LOW)  // button pressed
  {
    digitalWrite (LED, HIGH);  // turn led on
    dbg.println( "BUTTON LOW => LED HIGH" );
    dbg.println( "Timer1 started" );
    startTimer();
  }
  else  // button not pressed
  {
    digitalWrite (LED, LOW); // turned led off
    dbg.println( "BUTTON HIGH => LED LOW" );
    dbg.println( "Timer1 stopped" );
    Timer1.stop();
  }
}


