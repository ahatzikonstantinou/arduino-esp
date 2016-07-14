#include <SoftwareSerial.h>
SoftwareSerial esp(4,3);  // (Rx,Tx)

String SSID = "Wind WiFi 172839";      // change this to match your WiFi SSID
String PASS = "312ggp12";  // change this to match your WiFi password
String PORT = "80";           // using port 8080 by default

#define dbg Serial

void setup()
{
	dbg.begin(9600);
	dbg.println("-----------------------");
	dbg.println("begin.");

	//esp.begin(9600);
	//esp.setTimeout(5000);
}

byte stepAT = 0;
bool wifiSetupFinished = false; // set to true and stepAT to 0 in order to start DoSetupWifi
String stepATTerm = "";
String inputString = "";         // a string to hold incoming data

//values set in var stepATTermSearch, used by MySerialEvent
#define STEP_AT_TERM_SEARCHING 0
#define STEP_AT_TERM_FOUND 1
#define STEP_AT_TERM_NOT_FOUND 2
volatile byte stepATTermSearch = STEP_AT_TERM_SEARCHING;

// values returned by RunStep
#define STEP_AT_RUN_NOT_CURRENT -1
#define STEP_AT_RUN_RUNNING 0
#define STEP_AT_RUN_FAIL 1
#define STEP_AT_RUN_SUCCESS 2
#define STEP_AT_RUN_TIMEDOUT 3

bool done= false;
void loop()
{
// 	DoSetupWiFi();	
// 	MySerialEvent();
	if( !done )
	{
		done =true;
		MySplitString( "abc", '|' );
		MySplitString( "a|bc", '|' );
		MySplitString( "ab|c", '|' );
		MySplitString( "a|b|c", '|' );
		MySplitString( "|abc", '|' );
		MySplitString( "abc|", '|' );
	}
}

void MySplitString( String s, char delimit )
{
	dbg.println( "*************************" );
	dbg.print( "String: [" + s + "], delimit:" );
	dbg.println( delimit );

	int ind = s.indexOf( delimit );
	while( ind > -1 )
	{
		// dbg.println( "---------------" );
		String s1 = s.substring( 0, ind );
		// dbg.println( "s1: [" + s1 + "]" );
		dbg.println( s1 );
		String s2 = s.substring( ind + 1 );
		// dbg.println( "s2: [" + s2 + "]" );
		s = s2;
		ind = s.indexOf( delimit );
		if( ind == -1 )
		{
			dbg.println( s );
		}
	}
}

// By default we are looking for OK\r\n
//char OKrn[] = "OK\r\n";
char OKrn[] = "OK";

bool WaitForEspResponse(int timeout, char* term=OKrn) 
{
	unsigned long t = millis() + timeout;
	bool found = false;
	while( millis() < t && !found )
	{
		// found = esp.find(term);
		if( esp.available() )
		{
			dbg.print( (char) esp.read() );
		}
	}
	if( found )
	{
		dbg.print( "esp returned " );
	}
	else
	{
		dbg.print( "esp did not return " );
	}
	dbg.println( term );
	return true; //found;
}

bool stepStarted = false;
unsigned long stepATTimer = 0UL;
byte RunStep( byte step, String cmd, unsigned long timeout, char* term = OKrn )
{
	if( stepAT != step ) 
	{
		return STEP_AT_RUN_NOT_CURRENT;
	}

	if( !stepStarted )
	{
		dbg.print( "Ard: Started step (" );
		dbg.print( step );
		dbg.print( ") ending with term [" );
		dbg.print( term );
		dbg.print( "], cmd: {" );
		dbg.print( cmd );
		dbg.println( "}" );
		stepStarted = true;
		stepATTimer = millis();
		if( cmd.length() > 0 )
		{
			esp.println( cmd );
		}
		stepATTermSearch = STEP_AT_TERM_SEARCHING;
		stepATTerm = term;
		return STEP_AT_RUN_RUNNING;
	}

	if( stepATTermSearch == STEP_AT_TERM_SEARCHING )
	{
		unsigned long elapsedTime = millis() - stepATTimer;
		if( elapsedTime > timeout )
		{
			stepAT = stepAT+1;
			stepStarted = false;
			dbg.print( "Ard: Timed out after " );
			dbg.print( elapsedTime );
			dbg.print( " milliseconds, timeout: " );
			dbg.print( timeout );
			dbg.print( " (next step is " );
			dbg.print( stepAT );
			dbg.println( ")." );
			return STEP_AT_RUN_TIMEDOUT;
		}
		// dbg.println( "Still running" );
		return STEP_AT_RUN_RUNNING;
	}

	stepATTerm = "";
	stepAT = stepAT+1;
	stepStarted = false;
	if( stepATTermSearch == STEP_AT_TERM_NOT_FOUND )
	{
		dbg.print( "Ard: Fail (next step is " );
		dbg.print( stepAT );
		dbg.println( ")." );
		return STEP_AT_RUN_FAIL;
	}

	dbg.print( "Ard: Success (next step is " );
	dbg.print( stepAT );
	dbg.println( ")." );
	return STEP_AT_RUN_SUCCESS;
}

unsigned long DoSetupWiFiTimer = 0UL;
bool DoSetupWiFiStarted = false;
void DoSetupWiFi() 
{
	if( wifiSetupFinished )
	{
		return;
	}
	if( !DoSetupWiFiStarted )
	{
		DoSetupWiFiTimer = millis();
		DoSetupWiFiStarted = true;
		//while( esp.available() ); // eat leftover bytes lingering in serial, before issuing any AT commands
	}

	byte lastStepResult =	-1;
	// sometimes esp reconnects on its own, using the last known wifi connection
	if( RunStep( 0, "", 5000UL, "WIFI GOT IP" ) == STEP_AT_RUN_SUCCESS )
	{
		dbg.println( "Ard: DoSetupWiFi started with WIFI GOT IP. Will check if IP = '192.168.'" );
		if( RunStep( 1, "AT+CIFSR", 3000UL, "192.168." ) == STEP_AT_RUN_SUCCESS )
		{
			wifiSetupFinished = true;
			dbg.print( "Ard: DoSetupWiFi connected immediately. Finished in " );
			dbg.print( DoSetupWiFiTimer );
			dbg.println( " milliseconds." );
			return;
		}
	}

	// esp did not start by reconnecting to last known wifi, must setup
	RunStep( 1, "AT", 15000UL );
	RunStep( 2, "AT+CWMODE=1", 10000UL );
	// RunStep( 3, "", 5000UL, "" ); //just wait for 5000 millisecs
	RunStep( 3, "AT+CWJAP=\"" + SSID + "\",\"" + PASS + "\"", 15000UL, "OK" );
	RunStep( 4, "AT+CIPMUX=1", 10000UL );
	lastStepResult =	RunStep( 5, "AT+CIPSERVER=1," + PORT, 10000UL );
	// RunStep( STEP_AT_GMR, STEP_AT_CIFSR, "AT+GMR", 10000UL );
	// lastStepResult =	RunStep( STEP_AT_CIFSR, STEP_AT_NONE, "AT+CIFSR", 30000UL );
	if( lastStepResult == STEP_AT_RUN_SUCCESS || lastStepResult == STEP_AT_RUN_FAIL || lastStepResult == STEP_AT_RUN_TIMEDOUT )
	{
		DoSetupWiFiStarted = false;
		wifiSetupFinished = true;
		dbg.print( "Ard: DoSetupWiFi finished in " );
		dbg.print( ( millis() - DoSetupWiFiTimer ) );
		dbg.println( " milliseconds." );
	}
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void MySerialEvent() 
{
	unsigned long timeout = 0UL;
	int len = stepATTerm.length();
	do
	{
	  while( esp.available() )
	  {
	    // get the new byte:
	    char inChar = (char)esp.read();
	    // add it to the inputString:
	    inputString += inChar;
	    // dbg.println( "Ard: " + inChar );
	    // dbg.println( "Ard: inputString: " + inputString );
	    if( len > 0 && inputString.length() >= len ) 
	    {
				if( inputString.indexOf( stepATTerm ) > -1 )
	    	{	    		
	    		stepATTermSearch = STEP_AT_TERM_FOUND;
	    	}
	    }
			if( inChar == '\n' )
			{
				dbg.println( "Esp: " + inputString );
				inputString = "";
			}
	  }
		if( timeout == 0 )
		{
			timeout = millis() + 10;	//pkon: read any last remaining bytes
		}
	} while (millis() < timeout);
	if( len > 0 && stepATTermSearch == STEP_AT_TERM_FOUND )
	{
		dbg.println( "Ard: " + stepATTerm + " found!" );
	}
}