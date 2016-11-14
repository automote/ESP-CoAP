# ESP-CoAP
This repo contains CoAP protocol for operating ESP-12E as coap server. 

# CoAP server library for Arduino.
<a href="https://github.com/automote/ESP-CoAP" simple server library for Arduino.

## Source Code
This lightweight library source code are only 2 files. coap.cpp, coap.h.

## Example
Some sample sketches for Arduino included(/examples/).
 
 - coapserver.ino :simple server with endpoint url callback sample.

## How to use
Download this source code branch zip file and extract to the Arduino libraries directory or checkout repository.

    cd $HOME/Documents/Arduino/libraries/
    git clone https://github.com/automote/ESP-CoAP
    # restart Arduino IDE, you can find ESP-CoAP examples.
Upload the example code to ESP-12E,get the IPaddress of ESP-12E(server),run the coap client with following URI coap://IPaddress:default port number/resource to connect to ESP_12E server


