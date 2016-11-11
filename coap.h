
#ifndef __SIMPLE_COAP_H__
#define __SIMPLE_COAP_H__

#include <Arduino.h>
#include <WiFiUdp.h>

#define MAX_CALLBACK 10
#define COAP_HEADER_SIZE 4
#define COAP_OPTION_HEADER_SIZE 1
#define COAP_PAYLOAD_MARKER 0xFF
#define MAX_OPTION_NUM 10
#define BUF_MAX_SIZE 100
#define COAP_DEFAULT_PORT 5683
#define COAP_VERSION 1
#define MAX_AGE_DEFAULT 60

#define RESPONSE_CODE(class, detail) ((class << 5) | (detail))
#define COAP_OPTION_DELTA(v, n) (v < 13 ? (*n = (0xFF & v)) : (v <= 0xFF + 13 ? (*n = 13) : (*n = 14)))

typedef enum {
	COAP_CON = 0,
	COAP_NONCON = 1,
	COAP_ACK = 2,
	COAP_RESET = 3,
} COAP_TYPE;

typedef enum {
	COAP_EMPTY =0,
	COAP_GET = 1,
	COAP_POST = 2,
	COAP_PUT = 3,
	COAP_DELETE = 4,

} COAP_METHOD;

typedef enum {
	COAP_NULL=RESPONSE_CODE(0,0),
	COAP_CREATED = RESPONSE_CODE(2, 1),
	COAP_DELETED = RESPONSE_CODE(2, 2),
	COAP_VALID = RESPONSE_CODE(2, 3),
	COAP_CHANGED = RESPONSE_CODE(2, 4),
	COAP_CONTENT = RESPONSE_CODE(2, 5),
	COAP_BAD_REQUEST = RESPONSE_CODE(4, 0),
	COAP_UNAUTHORIZED = RESPONSE_CODE(4, 1),
	COAP_BAD_OPTION = RESPONSE_CODE(4, 2),
	COAP_FORBIDDEN = RESPONSE_CODE(4, 3),
	COAP_NOT_FOUND = RESPONSE_CODE(4, 4),
	COAP_METHOD_NOT_ALLOWD = RESPONSE_CODE(4, 5),
	COAP_NOT_ACCEPTABLE = RESPONSE_CODE(4, 6),
	COAP_PRECONDITION_FAILED = RESPONSE_CODE(4, 12),
	COAP_REQUEST_ENTITY_TOO_LARGE = RESPONSE_CODE(4, 13),
	COAP_UNSUPPORTED_CONTENT_FORMAT = RESPONSE_CODE(4, 15),
	COAP_INTERNAL_SERVER_ERROR = RESPONSE_CODE(5, 0),
	COAP_NOT_IMPLEMENTED = RESPONSE_CODE(5, 1),
	COAP_BAD_GATEWAY = RESPONSE_CODE(5, 2),
	COAP_SERVICE_UNAVALIABLE = RESPONSE_CODE(5, 3),
	COAP_GATEWAY_TIMEOUT = RESPONSE_CODE(5, 4),
	COAP_PROXYING_NOT_SUPPORTED = RESPONSE_CODE(5, 5)
} COAP_RESPONSE_CODE;

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
	COAP_PROXY_URI = 35,
	COAP_PROXY_SCHEME = 39
} COAP_OPTION_NUMBER;

typedef enum {
	COAP_NONE = -1,
	COAP_TEXT_PLAIN = 0,
	COAP_APPLICATION_LINK_FORMAT = 40,
	COAP_APPLICATION_XML = 41,
	COAP_APPLICATION_OCTET_STREAM = 42,
	COAP_APPLICATION_EXI = 47,
	COAP_APPLICATION_JSON = 50
} COAP_CONTENT_TYPE;



class CoapResource {
	public:
		String rt;
		uint8_t ct;
};

class CoapOption {
	public:
		uint8_t number;
		uint8_t length;
		uint8_t *buffer;
};

class CoapPacket {
	public:uint8_t version;
		uint8_t type;
		uint8_t code;
		uint8_t *token;
		uint8_t tokenlen;
		uint8_t *payload;
		uint8_t payloadlen;
		uint16_t messageid;
		uint8_t optionnum;
		CoapOption options[MAX_OPTION_NUM];

		void buffer_to_packet(uint8_t buffer[],int32_t packetlen);

		int parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);
                CoapPacket();




uint8_t version_();
uint8_t type_();
uint8_t tokenlen_();

uint8_t code_();

uint16_t messageid_();


uint8_t * token_();


};
typedef void (*callback)(CoapPacket *, IPAddress, int);

class CoapUri {
	private:
		String u[MAX_CALLBACK];
		callback c[MAX_CALLBACK];
	public:

		CoapUri();
		void add(callback call, String url,CoapResource resource[]);
		callback find(String url);

};


class Coap {


	public:
		Coap(

		    );

		bool start();
		bool start(int port);

		void server(callback c, String url);
		bool loop();


uint16_t sendPacket(CoapPacket *packet, IPAddress ip, int port);
void resourceDiscovery(CoapPacket *packet,IPAddress ip, int port,CoapResource resource[]);



uint16_t sendResponse(CoapPacket *packet,IPAddress ip, int port, char *payload);

uint16_t sendResponse(IPAddress ip, int port, uint16_t messageid,uint8_t type, char *payload, int payloadlen, COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE contentype, uint8_t *token, uint8_t tokenlen);

};

#endif
