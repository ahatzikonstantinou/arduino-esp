#ifndef mqtt_espsend_h
#define mqtt_espsend_h

#include "espsend.h"
#include "PubSubClient.h"

class MQTTEspSend : public EspSendClass
{
  public:
    void State();
    void Mode();
    void Status();
    void Name();

    void Finish();

    MQTTEspSend( PubSubClient & _client, String _topic );

  private:
    PubSubClient & client;
    static const int topicSize = 30;
    char topic[ topicSize ];
    static const int msgSize = 500;
    char msg[ msgSize ];
};

#endif
