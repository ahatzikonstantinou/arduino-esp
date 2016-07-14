/* ====== ESP8266 Demo ======
 * Print out analog values
 * (Updated Dec 14, 2014)
 * ==========================
 *
 * Change SSID and PASS to match your WiFi settings.
 * The IP address is displayed to soft serial upon successful connection.
 *
 * Ray Wang @ Rayshobby LLC
 * http://rayshobby.net/?p=9734
 */

// comment this part out if not using LCD debug

#define BUFFER_SIZE 512

#define SSID  "Wind WiFi 172839"      // change this to match your WiFi SSID
#define PASS  "312ggp12"  // change this to match your WiFi password
#define PORT  "80"           // using port 8080 by default

char buffer[BUFFER_SIZE];


int relayPin = 6;


// If using Software Serial for debug
// Use the definitions below
// #include <SoftwareSerial.h>
// SoftwareSerial esp(4,3);  // (Rx,Tx) use pins 7, 8 for software serial 
// #define dbg Serial
#define esp Serial
/*
// If your MCU has dual USARTs (e.g. ATmega644)
// Use the definitions below
#define dbg Serial    // use Serial for debug
#define esp Serial1   // use Serial1 to talk to esp8266
*/
// By default we are looking for OK\r\n
char OKrn[] = "OK\r\n";
byte wait_for_esp_response(int timeout, char* term=OKrn) {
  unsigned long t=millis();
  bool found=false;
  int i=0;
  int len=strlen(term);
  // wait for at most timeout milliseconds
  // or if OK\r\n is found
  while(millis()<t+timeout) {
    if(esp.available()) {
      buffer[i++]=esp.read();
      if(i>=len) {
        if(strncmp(buffer+i-len, term, len)==0) {
          found=true;
          break;
        }
      }
    }
  }
  buffer[i]=0;
  //dbg.print(buffer);
  return found;
}

void setup() {

  // assume esp8266 operates at 115200 baud rate
  // change if necessary to match your modules' baud rate
  esp.begin(9600); // 9600 57600 115200
  
  //dbg.begin(9600);
 //dbg.println("begin.");
    

  // print device IP address
//  //dbg.print("device ip addr:");
//  esp.println("AT+CIFSR");
//  wait_for_esp_response(1000);
  
  //setup relay
  setupRelay();
  //dbg.println("Relay setup finished");

//  SetupAP();

}

bool read_till_eol() {
  static int i=0;
  if(esp.available()) {
    buffer[i++]=esp.read();
    if(i==BUFFER_SIZE)  i=0;
    if(i>1 && buffer[i-2]==13 && buffer[i-1]==10) {
      buffer[i]=0;
      i=0;
      //dbg.print(buffer);
      return true;
    }
  }
  return false;
}
int done = 0;
void loop() {
  if (!done) {
    setupWiFi();
    done = 1;
  }
  
  int ch_id, packet_len;
  char *pb;  
  if(read_till_eol()) {
    if(strncmp(buffer, "+IPD,", 5)==0) {
      // request: +IPD,ch,len:data
      sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);
      if (packet_len > 0) {
        // read serial until packet_len character received
        // start from :
        pb = buffer+5;
        while(*pb!=':') pb++;
        pb++;
        if( strncmp(pb, "GET /open", 9 ) == 0) {
          wait_for_esp_response(1000);
          //dbg.println("received open.");
          switchRelay( true );
          //dbg.println("-> serve homepage");
          serve_homepage(ch_id, true, true);
        }
        else if( strncmp(pb, "GET /close", 10 ) == 0) {
          wait_for_esp_response(1000);
          //dbg.println("received close.");
          switchRelay( false );
          //dbg.println("-> serve homepage");
          serve_homepage(ch_id, true, false);
        }
        else if (strncmp(pb, "GET /", 5) == 0) {
          wait_for_esp_response(1000);
          //dbg.println("received:");
          //dbg.println(pb);
          //dbg.println("-> serve homepage");
          serve_homepage(ch_id, false, false);
        }
      }
    }
  }
}

void serve_homepage(int ch_id, boolean switchRelay, boolean relayOn) {
  //String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nRefresh: 5\r\n";
  String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n";

  String content="";
  if( switchRelay )
  {
    if( relayOn )
    {
      content += "Switched relay ON <br/>\n";
    }
    else
    {
      content += "Switched relay OFF <br/>\n";
    }    
  }
  else
  {
    // output the value of each analog input pin
    for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
      int sensorReading = analogRead(analogChannel);
      content += "analog input ";
      content += analogChannel;
      content += " is ";
      content += sensorReading;
      content += "<br />\n";       
    }
  }

  header += "Content-Length:";
  header += (int)(content.length());
  header += "\r\n\r\n";
  esp.print("AT+CIPSEND=");
  esp.print(ch_id);
  esp.print(",");
  esp.println(header.length()+content.length());
  if(wait_for_esp_response(2000, "> ")) {
    esp.print(header);
    esp.print(content);
  } else {
    //dbg.println("closing");
    esp.print("AT+CIPCLOSE=");
    esp.println(ch_id);
  }
}


void setupWiFi() {
  // try empty AT command
  esp.println("AT");
  wait_for_esp_response(1000);

  esp.println("AT+CWDHCP=1,1");
  wait_for_esp_response(1000);

  // set mode 1 (client)
  esp.println("AT+CWMODE=1");
  wait_for_esp_response(1000);  

  // reset WiFi module
  esp.print("AT+RST\r\n");
  wait_for_esp_response(1500);
  delay(3000);
 
  // join AP
  esp.print("AT+CWJAP=\"");
  esp.print(SSID);
  esp.print("\",\"");
  esp.print(PASS);
  esp.println("\"");
  // this may take a while, so wait for 5 seconds
  wait_for_esp_response(5000);
  
//  esp.println("AT+CIPSTO=30");  
//  wait_for_esp_response(1000);

  // start server
  esp.println("AT+CIPMUX=1");
  wait_for_esp_response(1000);
  
  esp.print("AT+CIPSERVER=1,"); // turn on TCP service
  esp.println(PORT);
  wait_for_esp_response(1000);
  
     
  // print firmware version
  //dbg.print( "Firmware: " );
  esp.println( "AT+GMR" );
  wait_for_esp_response(1000);
      
  // print device IP address
  //dbg.print("device ip addr:");
  esp.println("AT+CIFSR");
  wait_for_esp_response(5100);
}

/*
 * This function sets esp8266 to function as an Access Point.
 * See http://tech.scargill.net/esp8266-as-an-access-point/ for more info.
 */
#define AP_SSID "HA_AP"
#define AP_PASS "1234"
#define AP_CHAN 11
#define AP_SEC 3 // 0=no encryption (i.e. open), 1=WEP, 3=WPA_PSK, 3=WPA2_PSK and 4=WPA_WPA2_PSK

void SetupAP()
{
  // reset WiFi module
  esp.print("AT+RST\r\n");
  wait_for_esp_response(1500);
  delay(3000);
    
  // set mode 1 (access point)
  esp.println("AT+CWMODE=2");
  wait_for_esp_response(1000);

  esp.print("AT+RST\r\n");
  wait_for_esp_response(1500);
  delay(3000);
    
  // setup access point
//  esp.println( "AT+CWSAP=?") ;
//  esp.println( "AT+CIPSTA=?") ;
//  esp.println( "AT+CIPAP=?") ;
  esp.print( "AT+CWSAP=\"") ;
  esp.print( AP_SSID );
  esp.print( "\",\"" );
  esp.print( AP_PASS );
//  esp.println( "\"" );
  esp.print( "\"," );
  esp.println( AP_CHAN );
//  esp.print( "," );
//  esp.println( AP_SEC );
  wait_for_esp_response(1000);

  esp.print("AT+RST\r\n");
  wait_for_esp_response(1500);
  delay(3000);

  esp.println( "AT+CWSAP=?") ;    
  wait_for_esp_response(1500);
  delay(3000);

  // print device IP address
  //dbg.print("device ip addr:");
  esp.println("AT+CIFSR");
  wait_for_esp_response(1000);
}


void setupRelay() {
  pinMode(relayPin,OUTPUT);
  switchRelay( false );
  
}

void switchRelay( boolean on )
{
  if( on )
  {
    digitalWrite(relayPin, HIGH);
  }
  else
  {
    digitalWrite(relayPin, LOW);
  }
}
