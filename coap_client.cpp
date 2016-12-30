/*
This file is part of the ESP-COAP Client library for Arduino

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

*/

#include "coap_client.h"

//creating instance for required class
WiFiUDP udp;

//coap client begin
bool coapClient::start() {
	this->start(COAP_DEFAULT_PORT);
	return true;
}

bool coapClient::start(int port) {
	udp.begin(port);
	return true;
}

//get request
uint16_t coapClient::get(IPAddress ip, int port, char *url) {
	send(ip, port, url, COAP_CON, COAP_GET, NULL, 0, NULL, 0,0,NULL);
}

//put request
uint16_t coapClient::put(IPAddress ip, int port, char *url, char *payload, int payloadlen) {
	send(ip, port, url, COAP_CON, COAP_PUT, NULL, 0, (uint8_t *)payload, payloadlen,0,NULL);
}

//post request
uint16_t coapClient::post(IPAddress ip, int port, char *url, char *payload, int payloadlen) {
	send(ip, port, url, COAP_CON, COAP_POST, NULL, 0, (uint8_t *)payload, payloadlen,0,NULL);
}

//delete request
uint16_t coapClient::delet(IPAddress ip, int port, char *url){
	send(ip, port, url, COAP_CON, COAP_DELETE, NULL, 0, NULL, 0,0,NULL);


}

//ping
uint16_t coapClient::ping(IPAddress ip,int port){
	
	send (ip,port,NULL,COAP_CON,COAP_EMPTY,NULL,0,NULL,0,0,NULL);

}

//observe request
uint16_t coapClient::observe(IPAddress ip,int port,char *url,uint8_t optionbuffer){
	uint8_t token=rand();
	send(ip,port,url,COAP_CON,COAP_GET,&token,sizeof(token),NULL,0,COAP_OBSERVE,optionbuffer);

}

uint16_t coapClient::observeCancel(IPAddress ip,int port,char *url){
	send(ip,port,url,COAP_RESET,COAP_EMPTY,NULL,0,NULL,0,NULL,0);

}


uint16_t coapClient::send(IPAddress ip, int port, char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen,uint8_t number,uint8_t optionbuffer) {

	coapPacket packet;

	//make packet
	packet.type = type;
	packet.code = method;
	packet.token = token;
	packet.tokenlen = tokenlen;
	packet.payload = payload;
	packet.payloadlen = payloadlen;
	packet.optionnum = 0;
	packet.messageid = rand();

	if(number)
	{
		packet.options[packet.optionnum].buffer = &optionbuffer;
		packet.options[packet.optionnum].length = 0;
		packet.options[packet.optionnum].number = number;
		packet.optionnum++;
	}

	if(method!=COAP_EMPTY){

		// options
		packet.options[packet.optionnum].buffer = (uint8_t *)url;
		packet.options[packet.optionnum].length = strlen(url);
		packet.options[packet.optionnum].number = COAP_URI_PATH;
		packet.optionnum++;
	}
	// send packet
	sendPacket(packet, ip, port);
}

uint16_t coapClient::sendPacket(coapPacket &packet, IPAddress ip, int port) {

	uint8_t buffer[BUF_MAX_SIZE];
	uint8_t *p = buffer;
	uint16_t running_delta = 0;
	uint16_t packetSize = 0;


	*p = COAP_VERSION << 6;

	*p |= (packet.type & 0x03) << 4;

	*p++ |= (packet.tokenlen & 0x0F);


	*p++ = packet.code;
	*p++ = (packet.messageid >> 8);
	*p++ = (packet.messageid & 0xFF);
	p = buffer + COAP_HEADER_SIZE;
	packetSize += 4;

	// make token
	if (packet.token != NULL && packet.tokenlen <= 0x0F) {

		memcpy(p, packet.token, packet.tokenlen);
		p += packet.tokenlen;
		packetSize += packet.tokenlen;
	}

	// make option header
	for (int i = 0; i < packet.optionnum; i++)  {


		uint32_t optdelta;
		uint8_t len, delta;

		if (packetSize + 5 + packet.options[i].length >= BUF_MAX_SIZE) {
			return 0;
		}
		optdelta = packet.options[i].number - running_delta;
		COAP_OPTION_DELTA(optdelta, &delta);
		COAP_OPTION_DELTA((uint32_t)packet.options[i].length, &len);

		*p++ = (0xFF & (delta << 4 | len));
		if (delta == 13) {
			*p++ = (optdelta - 13);
			packetSize++;
		} else if (delta == 14) {
			*p++ = ((optdelta - 269) >> 8);
			*p++ = (0xFF & (optdelta - 269));
			packetSize+=2;
		} if (len == 13) {
			*p++ = (packet.options[i].length - 13);
			packetSize++;
		} else if (len == 14) {
			*p++ = (packet.options[i].length >> 8);
			*p++ = (0xFF & (packet.options[i].length - 269));
			packetSize+=2;
		}

		memcpy(p, packet.options[i].buffer, packet.options[i].length);
		p += packet.options[i].length;
		packetSize += packet.options[i].length + 1;
		running_delta = packet.options[i].number;
	}

	// make payload
	if (packet.payloadlen > 0) {

		if ((packetSize + 1 + packet.payloadlen) >= BUF_MAX_SIZE) {
			return 0;
		}
		*p++ = 0xFF;
		memcpy(p, packet.payload, packet.payloadlen);
		packetSize += 1 + packet.payloadlen;
	}


	udp.beginPacket(ip, port);
	udp.write(buffer, packetSize);
	udp.endPacket();

	return packet.messageid;
}

bool coapClient::loop() {

	uint8_t buffer[BUF_MAX_SIZE];
	int32_t packetlen = udp.parsePacket();

	while (packetlen > 0) {
		packetlen = udp.read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);

		coapPacket packet;

		// parse coap packet header
		if (packetlen < COAP_HEADER_SIZE || (((buffer[0] & 0xC0) >> 6) != 1)) {
			packetlen = udp.parsePacket();
			continue;
		}

		packet.type = (buffer[0] & 0x30) >> 4;
		packet.tokenlen = buffer[0] & 0x0F;
		packet.code = buffer[1];
		packet.messageid = 0xFF00 & (buffer[2] << 8);
		packet.messageid |= 0x00FF & buffer[3];

		if (packet.tokenlen == 0)  packet.token = NULL;
		else if (packet.tokenlen <= 8)  packet.token = buffer + 4;
		else {
			packetlen = udp.parsePacket();
			continue;
		}

		// parse packet options/payload
		if (COAP_HEADER_SIZE + packet.tokenlen < packetlen) {
			int optionIndex = 0;
			uint16_t delta = 0;
			uint8_t *end = buffer + packetlen;
			uint8_t *p = buffer + COAP_HEADER_SIZE + packet.tokenlen;
			while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
				packet.options[optionIndex];
				if (0 != parseOption(&packet.options[optionIndex], &delta, &p, end-p))
					return false;
				optionIndex++;
			}
			packet.optionnum = optionIndex;

			if (p+1 < end && *p == 0xFF) {
				packet.payload = p+1;
				packet.payloadlen = end-(p+1);
			} else {
				packet.payload = NULL;
				packet.payloadlen= 0;
			}
		}

		if (packet.type == COAP_ACK || packet.type ==  COAP_RESET) {
			// call response function
			resp(packet, udp.remoteIP(), udp.remotePort());

		} 

		return true;
	}

}

//parse option
int coapClient::parseOption(coapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
	uint8_t *p = *buf;
	uint8_t headlen = 1;
	uint16_t len, delta;

	if (buflen < headlen) return -1;

	delta = (p[0] & 0xF0) >> 4;
	len = p[0] & 0x0F;

	if (delta == 13) {
		headlen++;
		if (buflen < headlen) return -1;
		delta = p[1] + 13;
		p++;
	} else if (delta == 14) {
		headlen += 2;
		if (buflen < headlen) return -1;
		delta = ((p[1] << 8) | p[2]) + 269;
		p+=2;
	} else if (delta == 15) return -1;

	if (len == 13) {
		headlen++;
		if (buflen < headlen) return -1;
		len = p[1] + 13;
		p++;
	} else if (len == 14) {
		headlen += 2;
		if (buflen < headlen) return -1;
		len = ((p[1] << 8) | p[2]) + 269;
		p+=2;
	} else if (len == 15)
		return -1;

	if ((p + 1 + len) > (*buf + buflen))  return -1;
	option->number = delta + *running_delta;
	option->buffer = p+1;
	option->length = len;
	*buf = p + 1 + len;
	*running_delta += delta;

	return 0;
}





