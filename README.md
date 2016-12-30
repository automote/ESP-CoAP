# thingTronics ESP8266 12E Arduino Library
========================================
This is a Arduino Library for the ESP8266 12E.

# ESP-CoAP server library for Arduino
This repo contains CoAP protocol for operationg ESP-12E as server.

# IETF CoAP Draft
<a href="https://datatracker.ietf.org/doc/rfc7252/"target=_blank>CoAP</a> simple server library for Arduino.

## Repository Contents

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager.
* **library.json** - General library properties for the Arduino package manager in JSON format
* **keywords.txt** - Contains the keywords for Arduino IDE.

This lightweight library source code have only 2 files. coap.cpp, coap.h.

## Example
Some sample sketches for Arduino included(/examples/).

- coapserver.ino :simple server endpoint url callback sample.

## How to use
Download this source code branch zip file and extract to the Arduino libraries directory or checkout repository. Here is checkout on Ubuntu.

    cd $HOME/Downloads/Arduino/libraries/
    git clone https://github.com/automote/ESP-CoAP
    # restart Arduino IDE, you can find ESP-CoAP examples.

-Upload the example code to ESP-12E,get the IPaddress of ESP-12E(server).
-Run the coap client with following URI coap://IPaddress:default port number/resource ,to connect to ESP_12E server.

For more information about this library please vist <a href="https://github.com/automote/ESP-CoAP">here</a>.

## Where to Buy
You can buy the ESP-CoAP compatible modules from us by going to this URL <a href="thingtronics.com/products.html">here</a>.

### Features
- Working:
  - Methods
      - GET
      - PUT
      - POST (update working,creation not working)
      - DELETE (not working)
   - Observe
   - Resource Discovery 
   - Ping
   - Block Transfer (not working)
  
