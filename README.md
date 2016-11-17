# thingTronics LTR303 Arduino Library
========================================
This is a Arduino Library for the LTR303ALS Luminiosity sensor.
This illumination sensor has a flat response across most of the visible spectrum and has an adjustable integration time.

# ESP-CoAP server library for Arduino
This repo contains CoAP protocol for operationg ESP-12E as server.

# IETF CoAP Draft
<a href="http://coap.technology/" target=_blank>CoAP</a> simple server library for Arduino.

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
You can buy the ESP-CoAP compatible modules from us by going to this URL

thingtronics.com/products.html

### Features
- Working:
  - Methods
      - GET
      - PUT
      - POST  (not working)
      - DELETE ~~(not working)~~

#### Alpha Version
  - Alpha version is available for testing and verification
People who are interested can contact
