#include "mqtt_espsend.h"
#include "globals.h"
#include "my_wifi.h"

MQTTEspSend::MQTTEspSend( PubSubClient & _client, String _topic ):
  client( _client )
{
  _topic.toCharArray(topic, topicSize );
}

void MQTTEspSend::State()
{
  CreateHtmlState().toCharArray( msg, msgSize );
  client.publish( topic, msg );
}

void MQTTEspSend::Mode()
{
  CreateHtmlMode().toCharArray( msg, msgSize );
  client.publish( topic, msg );
}

void MQTTEspSend::Status()
{
  CreateHtmlStatus().toCharArray( msg, msgSize );
  client.publish( topic, msg );
}

void MQTTEspSend::Name()
{
  client.publish( topic, GetDeviceName() );
}

void MQTTEspSend::Finish()
{

}
