/*
 * ahat
 * nodemcu v1 demo
 */

#include <my_wifi.h>
#include <ultrasound.h>
#include <detect.h>
#include <globals.h>
#include <utils.h>
#include <button.h>
#include <seven_seg.h>
#include <EEPROM.h>

const char* ssid = "Wind WiFi 172839";
const char* password = "312ggp12";

// Create an instance of the server
// specify the port to listen on as an argument
// WiFiServer server(80);

void setup()
{
	Serial.begin(115200);
	delay(10);

	Serial.print("Connecting to ");
	Serial.println(ssid);

	// setup relay control pin
	// the BUILTIN_LED pin will be used to switch on/off the realy too
	pinMode( BUILTIN_LED, OUTPUT );

	// setup ultrasound sensor
	InitUltra();

	// setup button and mode of operation
	// the button is going to be an input
  // due to the internal pullup resistor the pin will be LOW when the button is pressed
  // and HIGH when the button is not pressed
  pinMode( BUTTON_PIN, INPUT_PULLUP );
	ChangeMode( MODE_ONE_WAVE );	// default mode of operation

	Switch( false ); // start with the relay switched off

	InitLeds();
	SetLeds();

	// setup wifi
	SetupWifi( ssid, password );

	// // Start the server
	// server.begin();
	// Serial.println("Server started");

	// Print the IP address
  Serial.println(WiFi.localIP());
}

int i=0;
int max_i = 3;
void loop()
{
	// DoHttpWiFi( server, Toggle, Switch );  // we always read wifi to be able to set mode from wifi no matter what current mode is

	ReadButton();

  if( GetMode() != MODE_WIFI )
  {
    if( GetMode() == MODE_PERSON )
    {
      ////dbg.println( "mode person" );
      if( !GetRelayOn() && DetectPersonPresent() == DetectPersonStatusIsPresent )
      {
        Switch( true );
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, "on" );
        //dbg.println( "Turned relay ON" );
      }
      else if( GetRelayOn() && DetectPersonAbsent() == DetectPersonStatusIsAbsent )
      {
        Switch( false );
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, "off" );
        //dbg.println( "Turned relay OFF" );
      }
    }
    else if( GetMode() == MODE_TWO_WAVE )
    {
      ////dbg.println( "mode wave" );
      if( DetectTwoWave() == TwoWaveDetected )
      {
        Toggle();
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, ( GetRelayOn() ? "on" : "off" ) );
      }
    }
    else if( GetMode() == MODE_ONE_WAVE )
    {
      ////dbg.println( "mode wave" );
      if( DetectOneWave() == OneWaveDetected )
      {
        Toggle();
        Log( LOG_ACTOR_ULTRASOUND, LOG_TARGET_RELAY, ( GetRelayOn() ? "on" : "off" ) );
      }
    }
  }

	delay( 100 );

	/*
	// Check if a client has connected
	WiFiClient client = server.available();
	if (!client)
	{
		return;
	}

	// Wait until the client sends some data
	Serial.println("new client");
	while(!client.available())
	{
		delay(1);
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	Serial.println(req);
	client.flush();

	// Match the request
	if (req.indexOf("/on") != -1)
	{
		mode = HIGH;
	}
	else if (req.indexOf("/off") != -1)
	{
		mode = LOW;
	}
	else if (req.indexOf("/tog") != -1)
	{
		if( mode == LOW )
		{
			mode = HIGH;
		}
		else
		{
			mode = LOW;
		}
	}
	else
	{
		Serial.println("invalid request");
		client.flush();
		client.stop();
		return;
	}

	client.flush();

	// Prepare the response
	String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
	s += (mode == HIGH)?"high":"low";
	s += "</html>\n";

	// Send the response to the client
	client.print(s);
	delay(1);
	Serial.println("Client disonnected");
	client.stop();


	digitalWrite( BUILTIN_LED, mode );
	// digitalWrite( BUILTIN_LED, LOW );
	// delay( i*100 );

	// digitalWrite( BUILTIN_LED, HIGH );
	// delay( i*100*2 );

	// i++;
	// if( i > max_i )
	// {
	// 	i = 0;
	// }

	Serial.print( "Ultra distance:" );
	// Serial.println( SampleUltra() );
	Serial.println( DblSampleUltra( MaxUltraDistance, MinUltraDistance, ULTRATRIG, ULTRAECHO ) );
	delay(1000);
	*/
}
