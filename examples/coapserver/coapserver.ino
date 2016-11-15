
#include <ESP8266WiFi.h>
#include <coap.h>

// CoAP server endpoint url callback
void callback_light(CoapPacket &packet, IPAddress ip, int port);


Coap coap;

const char* ssid = "IOT";
const char* password = "hasiot123";

// LED STATE
bool LEDSTATE;


// CoAP server endpoint URL
void callback_light(CoapPacket *packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");

  // send response
  char p[packet->payloadlen + 1];
  memcpy(p, packet->payload, packet->payloadlen);
  p[packet->payloadlen] = NULL;

  String message(p);

  if (message.equals("0"))
    LEDSTATE = false;
  else if (message.equals("1"))
    LEDSTATE = true;

  if (LEDSTATE) {
    digitalWrite(16, HIGH) ;
   coap.sendResponse(packet, ip, port, "1");
  } else {
    digitalWrite(16, LOW) ;
    coap.sendResponse(packet, ip, port, "0");
  }
}


void setup() {
  yield();
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println(" ");

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    yield();
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());

  // LED State
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  LEDSTATE = true;

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  //LEDSTATE = true;


  // add server url endpoints.
  // can add multiple endpoint urls.
  // exp) coap.server(callback_switch, "switch");
  //      coap.server(callback_env, "env/temp");
  //      coap.server(callback_env, "env/humidity");
 
  coap.server(callback_light, "light");
  coap.server(callback_lightled, "lightled");


  // start coap server/client
  coap.start();
 // coap.start(5683);
}

void loop() {
  coap.loop();
  delay(1000);
}
/*
  if you change LED, req/res test with coap-client(libcoap), run following.
  coap-client -m get coap://(arduino ip addr)/light
  coap-client -e "1" -m put coap://(arduino ip addr)/light
  coap-client -e "0" -m put coap://(arduino ip addr)/light
*/
