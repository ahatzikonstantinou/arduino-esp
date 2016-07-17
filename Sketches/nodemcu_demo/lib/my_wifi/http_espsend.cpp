#include "http_espsend.h"
#include "my_wifi.h"
#include "globals.h"

HttpEspSend::HttpEspSend( WiFiClient & _client ): client( _client )
{
  // client = &_client;
}

void HttpEspSend::State()
{
  EspSend( CreateHtmlState(), client );
}

void HttpEspSend::Mode()
{
  EspSend( CreateHtmlMode(), client );
}

void HttpEspSend::Status()
{
  EspSend( CreateHtmlStatus(), client );
}

void HttpEspSend::Name()
{
  EspSend( String( GetDeviceName() ), client );
}

void HttpEspSend::Finish()
{
  client.stop();
}
