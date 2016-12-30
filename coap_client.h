/*
This file is part of the ESP-COAP Client library for Arduino

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

*/

#ifndef __SIMPLE_COAP_H__
#define __SIMPLE_COAP_H__

#include <Arduino.h>
#include <WiFiUdp.h>

//current coap attributes
#define COAP_DEFAULT_PORT 5683
#define COAP_HEADER_SIZE 4
#define COAP_VERSION 1


//configuration
#define MAX_OPTION_NUM 10
#define BUF_MAX_SIZE 250

#define COAP_OPTION_DELTA(v, n) (v < 13 ? (*n = (0xFF & v)) : (v <= 0xFF + 13 ? (*n = 13) : (*n = 14)))

//coap message types
typedef enum {
	COAP_CON = 0,
	COAP_NONCON = 1,
	COAP_ACK = 2,
	COAP_RESET = 3
} COAP_TYPE;

//coap method values
typedef enum {
	COAP_EMPTY=0,
	COAP_GET = 1,
	COAP_POST = 2,
	COAP_PUT = 3,
	COAP_DELETE = 4
} COAP_METHOD;

//coap option values
typedef enum {
	COAP_IF_MATCH = 1,
	COAP_URI_HOST = 3,
	COAP_E_TAG = 4,
	COAP_IF_NONE_MATCH = 5,
	COAP_URI_PORT = 7,
	COAP_LOCATION_PATH = 8,
	COAP_URI_PATH = 11,
	COAP_CONTENT_FORMAT = 12,
	COAP_MAX_AGE = 14,
	COAP_URI_QUERY = 15,
	COAP_ACCEPT = 17,
	COAP_LOCATION_QUERY = 20,
	COAP_BLOCK_2=23,
	COAP_PROXY_URI = 35,
	COAP_PROXY_SCHEME = 39,
	COAP_OBSERVE=6
} COAP_OPTION_NUMBER;

//coap option class 
class coapOption {
	public:
		uint8_t number;
		uint8_t length;
		uint8_t *buffer;
};

//coap packet class
class coapPacket {
	public:
		uint8_t type;
		uint8_t code;
		uint8_t *token;
		uint8_t tokenlen;
		uint8_t *payload;
		uint8_t payloadlen;
		uint16_t messageid;
		uint8_t optionnum;
		coapOption options[MAX_OPTION_NUM];
};

typedef void (*callback)(coapPacket &, IPAddress, int);


class coapClient{
	public: 
		callback resp;
		bool start();
		bool start(int port);
		bool loop();
		uint16_t get(IPAddress ip, int port, char *url);
		uint16_t put(IPAddress ip, int port, char *url, char *payload,int payloadlen);
		uint16_t post(IPAddress ip, int port, char *url, char *payload,int payloadlen);
		uint16_t delet(IPAddress ip, int port, char *url);
		uint16_t ping(IPAddress ip, int port);
		uint16_t observe(IPAddress ip,int port,char *url,uint8_t observe); 

		uint16_t send(IPAddress ip, int port, char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen,uint8_t number,uint8_t buffer);

		uint16_t sendPacket(coapPacket &packet, IPAddress ip, int port);

		uint16_t observeCancel(IPAddress ip,int port,char *url);
		void response(callback c) { resp = c; }
		int parseOption(coapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);

};

#endif
