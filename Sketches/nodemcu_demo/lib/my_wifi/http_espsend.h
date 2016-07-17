#include "espsend.h"
#include <ESP8266WiFi.h>

class HttpEspSend : public EspSendClass
{
  public:
    void State();
    void Mode();
    void Status();
    void Name();

    void Finish();

    HttpEspSend( WiFiClient & _client );

  private:
     WiFiClient& client;
};
