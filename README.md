# thingTronics ESP8266 12E Arduino Library
========================================
This is a Arduino Library for the ESP8266 12E.

# ESP-CoAP server/client library for Arduino
This repo contains CoAP protocol for operationg ESP-12E as CoAP server and as CoAp client.

# IETF CoAP Draft
<a href="https://datatracker.ietf.org/doc/rfc7252/"target=_blank>CoAP</a> simple server library for Arduino.

## Repository Contents

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager.
* **library.json** - General library properties for the Arduino package manager in JSON format
* **keywords.txt** - Contains the keywords for Arduino IDE.

This lightweight library source code have only 4 files. coapServer.cpp, coapServer.h, coapClient.cpp, coapClient.h .

## Example
Some sample sketches for Arduino included(/examples/).

- coapserver.ino :simple server endpoint url callback sample.
- coapclient.ino :simple client response callback sample.

## How to use
Download this source code branch zip file and extract to the Arduino libraries directory or checkout repository. Here is checkout on Ubuntu.

    cd $HOME/Downloads/Arduino/libraries/
    git clone https://github.com/automote/ESP-CoAP
    # restart Arduino IDE, you can find ESP-CoAP examples.

###working with CoAP server
-Upload the server example code to ESP-12E,get the IPaddress of ESP-12E(server).
-Run the coap client(web browser) with following URI coap://IPaddress:default port number/resource ,to connect to ESP_12E server.
###working with CoAP client
-Upload the client example cose to ESP-12E,check the working by taking ETH Zurich as server.

For more information about this library please vist <a href="https://github.com/automote/ESP-CoAP">here</a>.

## Where to Buy
You can buy the ESP-CoAP compatible modules from us by going to this URL <a href="thingtronics.com/products.html">here</a>.

### Features
- Server Side Working:
  - Methods
      - GET
      - PUT
      - POST (update working,creation not working)
      - DELETE (not working)
   - Ping
   - Observe (only 1 resource; only 10 observers)
   - Resource Discovery 
   - Block Transfer (NA)

- Client Side Working:
  - Methods
      - GET
      - PUT
      - POST 
      - DELETE 
   - Observe
   - Ping 
   
## Maintainers

The ESP-CoAP is maintained by thingTronics Innovations.

Main contributor:
 * Poornima Nagesh @<poornima.nagesh@thingtronics.com>
 * Lovelesh Patel @<lovelesh.patel@thingtronics.com>
