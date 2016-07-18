#include "my_wifi.h"
#include "seven_seg.h"
#include <globals.h>
#include <utils.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient, http://pubsubclient.knolleary.net/, http://pubsubclient.knolleary.net/api.html
#include "espsend.h"
#include "http_espsend.h"
#include "mqtt_espsend.h"

byte wifiState = WIFI_STATE_NOT_SETUP; // set to WIFI_STATE_NOT_SETUP

/*
* Auxilliary definitions
*/
#define LOG_SERVER_TCP_PORT "8000"
#define LOG_SERVER_IP_ADDRESS "192.168.1.4"

const char* mqtt_server = "192.168.1.1"; //  HomePC

WiFiClient espClient;
PubSubClient client(espClient);

/*
* Auxilliary functions
*/
void ProcessRequest( String req, EspSendClass& espsend );

void Log( byte actor, byte target, String value )
{
  Serial.println( F("Ard: will Log.") );
  String content = String( "device: " ) + GetDeviceName();
  content += ", actor: ";
  switch( actor )
  {
    case LOG_ACTOR_WIFI:
    content += "wifi";
    break;
    case LOG_ACTOR_BUTTON:
    content += "button";
    break;
    case LOG_ACTOR_ULTRASOUND:
    content += "ultrasound";
    break;
    case LOG_ACTOR_SYSTEM:
    content += "system";
    break;
  }
  content += ", target: ";
  switch( target )
  {
    case LOG_TARGET_RELAY:
    content += "relay";
    break;
    case LOG_TARGET_MODE:
    content += "GetMode()";
    break;
    case LOG_TARGET_NAME:
    content += "name";
    break;
    case LOG_TARGET_SYSTEM:
    content += "system";
    break;
  }
  content += ", time: " + String( millis() ) + ", value: " + value;

  Serial.print( "Logging: " );
  Serial.println( content );
  //TODO send to log server
}

void EspSend( String content, WiFiClient& client )
{
  Serial.print( F("Ard: will EspSend [") );
  Serial.print( content );
  Serial.println( F("]") );

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += content;
  s += "\r\n</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
  client.stop();
}

String CreateHtmlState()
{
  String content="Relay ";
  if( GetRelayOn() )
  {
    content += "ON" ;
  }
  else
  {
    content += "OFF" ;
  }
  return content;
}

String CreateHtmlMode()
{
  String content = "Mode ";
  if( GetMode() == MODE_PERSON )
  {
    content += "person";
  }
  else if ( GetMode() == MODE_TWO_WAVE )
  {
    content += "two-wave";
  }
  else if ( GetMode() == MODE_ONE_WAVE )
  {
    content += "one-wave";
  }
  else if ( GetMode() == MODE_WIFI )
  {
    content += "wifi";
  }
  return content;
}

String CreateHtmlStatus()
{
  String content = CreateHtmlState();
  content += "<br/>\n";
  content += CreateHtmlMode();
  return content;
}

void EspSendMode( WiFiClient& client )
{
  EspSend( CreateHtmlMode(), client );
}

void EspSendStatus( WiFiClient& client )
{
  EspSend( CreateHtmlStatus(), client);
}

void EspSendState( WiFiClient& client )
{
  EspSend( CreateHtmlState(), client );
}

void EspSendName( WiFiClient& client )
{
  EspSend( GetDeviceName(), client );
}


void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
* End of Auxilliary functions
*/


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String msg = String( ( char * ) payload );

  String mqttTopic = String( "home/%s" ) + GetDeviceName();
  EspSendClass espsend = MQTTEspSend( client, mqttTopic );
  ProcessRequest( msg, espsend );
}

/*
* API functions
*/

void StartMQTT()
{
  client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void SetupWifi( const char* ssid, const char* password )
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    wifiState = WIFI_STATE_SETTING_UP;
    SetLeds();
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  wifiState = WIFI_STATE_OK;
  SetLeds();
}

void DoHttpWiFi( WiFiServer& server, void (*Toggle)(), void (*Switch)( bool ) )
{
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

  EspSendClass espsend = HttpEspSend( client );
  ProcessRequest( req, espsend );
}

void DoMQTT()
{
  if( !client.connected() )
  {
    reconnect();
  }
  client.loop();
}

void ProcessRequest( String req, EspSendClass& espsend )
{
  // Match the request
  if( req.indexOf( "GET /on " ) > -1 )
  {
    Serial.println( F("Ard: Will switch ON."));
    Switch( true );
    espsend.State();
    // EspSendState( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_RELAY, "on" );
  }
  else if( req.indexOf( "GET /off " ) > -1 )
  {
    Serial.println( F("Ard: Will switch OFF."));
    Switch( false );
    espsend.State();
    // EspSendState( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_RELAY, "off" );
  }
  else if( req.indexOf( "GET /person " ) > -1 )
  {
    Serial.println( F("Ard: Will set mode to PERSON."));
    ChangeMode( MODE_PERSON );
    espsend.Mode();
    // EspSendMode( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "person" );
  }
  else if( req.indexOf( "GET /one_wave " ) > -1 )
  {
    Serial.println( F("Ard: Will set mode to ONE WAVE."));
    ChangeMode( MODE_ONE_WAVE );
    espsend.Mode();
    //EspSendMode( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "one_wave" );
  }
  else if( req.indexOf( "GET /two_wave " ) > -1 )
  {
    Serial.println( F("Ard: Will set mode to TWO WAVE."));
    ChangeMode( MODE_TWO_WAVE );
    espsend.Mode();
    //EspSendMode( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "two_wave" );
  }
  else if( req.indexOf( "GET /wifi " ) > -1 )
  {
    Serial.println( F("Ard: Will set mode to WIFI."));
    ChangeMode( MODE_WIFI );
    espsend.Mode();
    //EspSendMode( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_MODE, "wifi" );
  }
  else if( req.indexOf( "GET /mode " ) > -1 )
  {
    Serial.println( F("Ard: Will return mode."));
    espsend.Mode();
    //EspSendMode( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "mode_query" );
  }
  else if( req.indexOf( "GET /state " ) > -1 )
  {
    Serial.println( F("Ard: Will return state (ON/OFF)."));
    espsend.State();
    //EspSendState( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "state_query" );
  }
  else if( req.indexOf( "GET /status " ) > -1 )
  {
    Serial.println( F("Ard: Will return status (mode and state)."));
    espsend.Status();
    //EspSendStatus( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "status_query" );
  }
  else if( req.indexOf( "GET /name " ) > -1 )
  {
    Serial.println( F("Ard: Will return name."));
    espsend.Name();
    //EspSendName( client );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_SYSTEM, "name_query" );
  }
  else if( req.indexOf( "GET /name=" ) > -1 )
  {
    int ind = req.indexOf( "GET /name=" );
    int end = req.indexOf( " ", ind + 1 );
    String name = GetDeviceName();
    if( end > -1 )
    {
      name = req.substring( ind, end );
    }
    else
    {
      name = req.substring( ind);
    }
    Serial.print( F("Ard: Will set name to [") );
    Serial.print( name );
    Serial.println( F("].") );
    UpdateDeviceName( name );
    Log( LOG_ACTOR_WIFI, LOG_TARGET_NAME, "name" );
  }
  else
  {
    Serial.println("invalid request");
    // 	client.flush();
    espsend.Finish();
    //client.stop();
    return;
  }

  // 	client.flush();
  espsend.Finish();
  //client.stop();
}

byte GetWifiState()
{
  return wifiState;
}
