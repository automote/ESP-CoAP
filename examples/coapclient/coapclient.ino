/*
ESP-COAP Client
*/
#include <ESP8266WiFi.h>
#include "coap_client.h"

//instance for coapclient
coapClient coap;

//WiFi connection info
const char* ssid = "******";
<<<<<<< HEAD
const char* password = "*********";
=======
const char* password = "*******";
>>>>>>> observe

//ip address and default port of coap server in which your interested in
IPAddress ip(129,132,15,80);//take ETH Zurich or coap.me server to run and check client 
int port =5683;

// coap client response callback
void callback_response(coapPacket &packet, IPAddress ip, int port);

// coap client response callback
void callback_response(coapPacket &packet, IPAddress ip, int port) {
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    //response from coap server
 if(packet.type==3 && packet.code==0){
      Serial.println("ping ok");
    }

    Serial.println(p);
}

void setup() {
   
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.println(" ");

    // Connection info to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    yield();
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    // Print the IP address of client
    Serial.println(WiFi.localIP());

    // client response callback.
    // this endpoint is single callback.
    coap.response(callback_response);

    // start coap client
    coap.start();

    //get request to server (arguments ip adrress of server,default port,resource(uri))
    int msgid = coap.get(ip,port,"light");

    //observe request (arguments ip adrress of server,deafult port,resource name,interger(0) ) 
    //int msgid= coap.observe(ip,port,"light",0);

    //reset observe cancel
    //int msgid=coap.observecancel(ip,port,"resoucename");
    
}
//int i=0;

void loop() {
    bool state;
 
    // Requests

    //get request
    //int msgid = coap.get(ip,port,"hello");

    //put request 
    //arguments server ip address,default port,resource name, payload,payloadlength
    //int msgid =coap.put(ip,port,"resourcename","0",strlen("0"));

    //post request
    //arguments server ip address,default port,resource name, payload,payloadlength
    //int msgid =coap.post(ip,port,"resourcename","0",strlen("0"));

    //delete request
    //int msgid = coap.delet(ip,port,"resourcename");

    //ping
    //int msgid=coap.ping(ip,port);
    
    // int msgid=coap.observe(ip,port,"obs",0);
  
    state= coap.loop();
    // Serial.print("state=");
    //Serial.println(state);
    //if(state==true)
          //i=i+1;
  
    //Serial.print("i=");
    //Serial.println(i);
      //if(i==3)
    //{
        //Serial.println("cancel observe");
        //coap.observeCancel(ip,port,"resourcename");
    //}
  
    //Serial.println(msgid);
    delay(1000);
}
