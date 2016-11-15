# ESP-CoAP server library for Arduino
This repo contains CoAP protocol for operationg ESP-12E as server.

# CoAP server library for Arduino.
<a href="http://coap.technology/" target=_blank>CoAP</a> simple server library for Arduino.

## Source Code
This lightweight library source code are only 2 files. coap.cpp, coap.h.

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
