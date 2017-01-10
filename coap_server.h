/*
This file is part of the ESP-COAP Server library for Arduino

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
#define COAP_VERSION 1
#define COAP_HEADER_SIZE 4
#define COAP_OPTION_HEADER_SIZE 1
#define COAP_PAYLOAD_MARKER 0xFF

//configuration
#define MAX_OPTION_NUM 10
#define BUF_MAX_SIZE 250
#define MAX_CALLBACK 10
#define MAX_AGE_DEFAULT 60
#define MAX_OBSERVER 10

#define COAP_OPTION_DELTA(v, n) (v < 13 ? (*n = (0xFF & v)) : (v <= 0xFF + 13 ? (*n = 13) : (*n = 14)))

//coap message types
typedef enum {
	COAP_CON = 0,
	COAP_NONCON = 1,
	COAP_ACK = 2,
	COAP_RESET = 3,
} COAP_TYPE;

//coap method values
typedef enum {
	COAP_EMPTY =0,
	COAP_GET = 1,
	COAP_POST = 2,
	COAP_PUT = 3,
	COAP_DELETE = 4,
} COAP_METHOD;

//coap response values
typedef enum {
	COAP_EMPTY_MESSAGE=0,
	COAP_CREATED =65,
	COAP_DELETED = 66,
	COAP_VALID = 67,
	COAP_CHANGED = 68,
	COAP_CONTENT = 69,
	COAP_BAD_REQUEST = 128,
	COAP_UNAUTHORIZED = 129,
	COAP_BAD_OPTION = 130,
	COAP_FORBIDDEN = 131,
	COAP_NOT_FOUND =132,
	COAP_METHOD_NOT_ALLOWD =133,
	COAP_PRECONDITION_FAILED = 140,
	COAP_REQUEST_ENTITY_TOO_LARGE=141,
	COAP_UNSUPPORTED_CONTENT_FORMAT = 143,
	COAP_INTERNAL_SERVER_ERROR = 160,
	COAP_NOT_IMPLEMENTED = 161,
	COAP_BAD_GATEWAY = 162,
	COAP_SERVICE_UNAVALIABLE =163,
	COAP_GATEWAY_TIMEOUT = 164,
	COAP_PROXYING_NOT_SUPPORTED = 165
} COAP_RESPONSE_CODE;

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

//coap content format types
typedef enum {

	COAP_TEXT_PLAIN = 0,
	COAP_APPLICATION_LINK_FORMAT = 40,
	COAP_APPLICATION_XML = 41,
	COAP_APPLICATION_OCTET_STREAM = 42,
	COAP_APPLICATION_EXI = 47,
	COAP_APPLICATION_JSON = 50
} COAP_CONTENT_TYPE;

//coap class::used for resource discovery request
class resource_dis {
	public:
		String rt;
		uint8_t ct;
		String title;
};

//coap option class
class coapOption {
	public:
		uint8_t number;
		uint8_t length;
		uint8_t *buffer;
};

//coap packet class
class coapPacket {
	public:uint8_t version;
	       uint8_t type;
	       uint8_t code;
	       uint8_t *token;
	       uint8_t tokenlen;
	       uint8_t *payload;
	       uint8_t payloadlen;
	       uint16_t messageid;
	       uint8_t optionnum;
	       coapOption options[MAX_OPTION_NUM];

	       void bufferToPacket(uint8_t buffer[],int32_t packetlen);

	       int parseOption(coapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);
	       coapPacket();

	       uint8_t version_();
	       uint8_t type_();
	       uint8_t tokenlen_();
	       uint8_t code_();
	       uint16_t messageid_();
	       uint8_t * token_();
};

typedef void (*callback)(coapPacket *, IPAddress, int,int);


class coapUri {
	public:
		String u[MAX_CALLBACK];
		callback c[MAX_CALLBACK];

		coapUri();
		void add(callback call, String url,resource_dis resource[]);
		callback find(String url);
};

//coap class::used for maintaining the details of clients making observe request 
class coapObserver{
	public:
		uint8_t* observer_token;
		uint8_t observer_tokenlen;
		IPAddress observer_clientip;
		int observer_clientport;
		String observer_url;
};

//coap class
class coapServer {
	public:
		coapServer( );

		bool start();
		bool start(int port);

		void server(callback c, String url);
		bool loop();

		uint16_t sendPacket(coapPacket *packet, IPAddress ip, int port);
		void resourceDiscovery(coapPacket *packet,IPAddress ip, int port,resource_dis resource[]);

		void sendResponse(IPAddress ip, int port, char *payload);
		void addObserver(String url,coapPacket *request,IPAddress ip,int port);
		void sendResponse(char *payload);
		void notification(char *payload);

};

#endif
