/*
This file is part of the ESP-COAP Server library for Arduino

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

*/

#include "coap_server.h"

//creating instance for required class
WiFiUDP Udp;

coapUri uri;
resource_dis resource[MAX_CALLBACK];
coapPacket *request=new coapPacket();
coapPacket *response=new coapPacket();
coapObserver observer[MAX_OBSERVER];

//counter for maintaining resource count
static uint8_t rcount=0;
//counter for maintaing observer count
static uint8_t obscount=0;
//attributes of millis
unsigned long interval =1000;
unsigned long previousMillis=0;
//attributes of observe request    
static uint8_t obsstate=10;
char *previousPayload="";
uint16_t messid=200;

//constructor of coapuri class
coapUri::coapUri() {
	for (int i = 0; i < MAX_CALLBACK; i++) {
		u[i] = "";
		c[i] = NULL;
	}
}

//adding resources 
void coapUri::add(callback call, String url,resource_dis resource[]) {

	for (int i = 0; i < MAX_CALLBACK; i++)
		if (c[i] != NULL && u[i].equals(url)) {
			c[i] = call;
			rcount++;
			resource[i].rt=url;
			resource[i].ct=0;
			if(i==0)
				//resource[i].title="observable resource";	
				return ;
		}

	for (int i = 0; i < MAX_CALLBACK; i++) 
		if (c[i] == NULL) {
			c[i] = call;
			u[i] = url;
			rcount++;
			resource[i].rt=url;
			resource[i].ct=0;

			return;
		}
}

//finding request url(resource)
callback coapUri::find(String url) {
	for (int i = 0; i < MAX_CALLBACK; i++) if (c[i] != NULL && u[i].equals(url)) return c[i];
	return NULL;
}

void coapServer::server(callback c, String url) {
	uri.add(c, url,resource); 
}

//constructor of coap class
coapServer::coapServer() {
}

//coap server begin
bool coapServer::start() {
	this->start(COAP_DEFAULT_PORT);
	return true;
}

bool coapServer::start(int port) {
	Udp.begin(port);
	return true;
}

//constructor of coappacket class
coapPacket::coapPacket(){
}

uint8_t coapPacket::version_(){
	return version;
}

uint8_t coapPacket::type_(){
	return type;
}

uint8_t coapPacket::code_(){
	return code;
}

uint16_t coapPacket::messageid_(){
	return messageid;
}

uint8_t * coapPacket::token_(){
	return token;
}

//parse option
int coapPacket::parseOption(coapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {

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

bool coapServer::loop() {

	uint8_t buffer[BUF_MAX_SIZE];
	int32_t packetlen = Udp.parsePacket();

	if (packetlen > 0) {

		packetlen = Udp.read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);

		request->bufferToPacket(buffer,packetlen);

		//call endpoint url function

		String url = "";
		for (int i = 0; i < request->optionnum; i++) {
			if (request->options[i].number == COAP_URI_PATH && request->options[i].length > 0) {
				char urlname[request->options[i].length + 1];
				memcpy(urlname,request->options[i].buffer,request->options[i].length);
				urlname[request->options[i].length] = NULL;
				if(url.length() > 0)
					url += "/";
				url += urlname;

			}
		}


		//response


		if(request->code_()==COAP_EMPTY && request->type_()== COAP_CON ){

			response->version=request->version;
			response->type=COAP_RESET;
			response->code=COAP_EMPTY_MESSAGE;
			response->messageid=request->messageid;
			response->token=request->token;
			response->payload=NULL;
			response->payloadlen=0;
			sendPacket(response,Udp.remoteIP(),Udp.remotePort());
		}else if(request->code_()==COAP_EMPTY && request->type_()==COAP_RESET){

			for(uint8_t i=0;i<obscount;i++){
				if(observer[i].observer_clientip==Udp.remoteIP()  && observer[i].observer_url==url){

					observer[i]=observer[i+1];
					observer[i+1]={0};
					obscount=obscount-1;
				}
			}

		}
		else if(request->code_()==COAP_GET||request->code_()==COAP_PUT||request->code_()==COAP_POST||request->code_()==COAP_DELETE){

			if(request->type_()== COAP_CON){

				response->version=request->version;
				response->type=COAP_ACK;
				response->tokenlen=request->tokenlen;
				response->messageid=request->messageid;
				response->token=request->token;
			}
			else if (request->type_()==COAP_NONCON){
				response->version=request->version;
				response->type=COAP_NONCON;
				response->tokenlen=request->tokenlen;
				response->messageid=request->messageid;
				response->token=request->token;
			}

			if(request->code_()==COAP_GET){

				uint8_t num;
				for(uint8_t i=0;i<=request->optionnum;i++)
				{
					if(request->options[i].number==COAP_OBSERVE){
						num=i;
						break;
					}
				}



				if(request->options[num].number==COAP_OBSERVE){

					if(*(request->options[num].buffer)==1){

						for(uint8_t i=0;i<obscount;i++){
							if(observer[i].observer_clientip==Udp.remoteIP()  && observer[i].observer_url==url){

								observer[i]=observer[i+1];
								observer[i+1]={0};
								obscount=obscount-1;
							}

						}

						uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);

					}

					else {

						addObserver(url,request,Udp.remoteIP(),Udp.remotePort());

					}
				}
				else if(url==String(".well-known/core")){

					resourceDiscovery(response,Udp.remoteIP(),Udp.remotePort(),resource);

				}else if(!uri.find(url)){


					response->payload=NULL;
					response->payloadlen=0;
					response->code=COAP_NOT_FOUND;

					response->optionnum=0;

					char optionBuffer[2];
					optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
					optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
					response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
					response->options[response->optionnum].length = 2;
					response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
					response->optionnum++;

					sendPacket(response,Udp.remoteIP(),Udp.remotePort());	

				}else{

					uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);
				}

			}else if(request->code_()==COAP_PUT){

				if(!uri.find(url)){

					response->payload=NULL;
					response->payloadlen=0;
					response->code=COAP_NOT_FOUND;


					response->optionnum=0;

					char optionBuffer[2];
					optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
					optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
					response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
					response->options[response->optionnum].length = 2;
					response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
					response->optionnum++;

					sendPacket(response,Udp.remoteIP(),Udp.remotePort());	

				}else{
					uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);
				}
			}else if(request->code==COAP_POST){

				int i;
				for( i=0;i<rcount;i++){
					if(resource[i].rt==url){

						uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);
						break;
					}
				}
				if(i==rcount){
					//add new resource

				}

			}else if(request->code==COAP_DELETE){


				if(!uri.find(url)){
					response->payload=NULL;
					response->payloadlen=0;
					response->code=COAP_NOT_FOUND;

					response->optionnum=0;

					char optionBuffer[2];
					optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
					optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
					response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
					response->options[response->optionnum].length = 2;
					response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
					response->optionnum++;

					sendPacket(response,Udp.remoteIP(),Udp.remotePort());	

				}else{//delete

				}

			}
		}

	}

	//checking for the change for resource 
	unsigned currentMillis=millis();
	if ((unsigned long)(currentMillis-previousMillis)>=interval)
	{	
		//observing a resouce 

		uri.find(resource[0].rt)(request,(0,0,0,0),NULL,1);
		previousMillis=millis();
	}


}

void coapPacket::bufferToPacket(uint8_t buffer[],int32_t packetlen){

	//parse coap packet header
	version=(buffer[0] & 0xC0)>>6;
	type= (buffer[0] & 0x30) >> 4;
	tokenlen = buffer[0] & 0x0F;
	code = buffer[1];
	messageid = 0xFF00 & (buffer[2] << 8);
	messageid |= 0x00FF & buffer[3];
	if (tokenlen == 0)  token = NULL;
	else if (tokenlen <= 8) 
	{       token=new uint8_t(tokenlen);
		memset(token,0,tokenlen);
		for(int i=0;i<tokenlen ;i++)
		{
			token[i]=buffer[4+i];
		}

	}
	else {
		packetlen =Udp.parsePacket();

	}

	//parse packet options/payload
	if (COAP_HEADER_SIZE +tokenlen < packetlen) {
		int optionIndex = 0;
		uint16_t delta = 0;
		uint8_t *end = buffer + packetlen;
		uint8_t *p = buffer + COAP_HEADER_SIZE+tokenlen;

		while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {          

			options[optionIndex];
			if (0 ==parseOption(&options[optionIndex], &delta, &p, end-p))

				optionIndex++;

		}
		optionnum = optionIndex;


		if (p+1 < end && *p == 0xFF) {
			payload = p+1;
			payloadlen = end-(p+1);
		} else {

			payload = NULL;
			payloadlen= 0;
		}
	}


}

uint16_t coapServer::sendPacket(coapPacket *packet, IPAddress ip, int port) {

	uint8_t buffer[BUF_MAX_SIZE];
	uint8_t *p = buffer;
	uint16_t running_delta = 0;
	uint16_t packetSize = 0;

	//make coap packet base header
	*p = (packet->version )<< 6;
	*p |= (packet->type & 0x03) << 4;
	*p++ |= (packet->tokenlen & 0x0F);
	*p++ = packet->code;
	*p++ = (packet->messageid >> 8);
	*p++ = (packet->messageid & 0xFF);
	p = buffer + COAP_HEADER_SIZE;
	packetSize += 4;

	// make token
	if (packet->token != NULL && packet->tokenlen <=8) {

		memcpy(p, packet->token, packet->tokenlen);
		p += packet->tokenlen;
		packetSize += packet->tokenlen;
	}

	// make option header
	for (int i = 0; i < packet->optionnum; i++)  {

		uint32_t optdelta;
		uint8_t len, delta;

		if (packetSize + 5 + packet->options[i].length >= BUF_MAX_SIZE) {
			return 0;
		}
		optdelta = packet->options[i].number - running_delta;
		COAP_OPTION_DELTA(optdelta, &delta);
		COAP_OPTION_DELTA((uint32_t)packet->options[i].length, &len);

		*p++ = (0xFF & (delta << 4 | len));
		if (delta == 13) {
			*p++ = (optdelta - 13);
			packetSize++;
		} else if (delta == 14) {
			*p++ = ((optdelta - 269) >> 8);
			*p++ = (0xFF & (optdelta - 269));
			packetSize+=2;
		} if (len == 13) {
			*p++ = (packet->options[i].length - 13);
			packetSize++;
		} else if (len == 14) {
			*p++ = (packet->options[i].length >> 8);
			*p++ = (0xFF & (packet->options[i].length - 269));
			packetSize+=2;
		}

		memcpy(p, packet->options[i].buffer, packet->options[i].length);
		p += packet->options[i].length;
		packetSize += packet->options[i].length + 1;
		running_delta = packet->options[i].number;
	}

	// make payload
	if (packet->payloadlen > 0) {

		if ((packetSize + 1 + packet->payloadlen) >= BUF_MAX_SIZE) {
			return 0;
		}
		*p++ = 0xFF;
		memcpy(p, packet->payload, packet->payloadlen);
		packetSize += 1 + packet->payloadlen;
	}

	Udp.beginPacket(ip, port);
	Udp.write(buffer, packetSize);
	Udp.endPacket();

}

//resource discovery
void coapServer::resourceDiscovery(coapPacket *response,IPAddress ip, int port,resource_dis resource[])
{

	String str_res;

	for(int i=0;i<rcount;i++)
	{   
		str_res +="</";
		str_res +=resource[i].rt;
		str_res +=">;";
		str_res +=resource[i].rt;
		str_res +=";rt=";
		str_res +="\"";
		str_res +="observe";
		str_res +="\"";

		str_res +=";";
		str_res +="ct=";
		str_res +=resource[i].ct;
		str_res +=";";
		if(i==0){str_res+="title=\"";
			str_res +="observable resource";
			str_res+="\"";
		}
		str_res +=",";
	}

	const char *payload=str_res.c_str();

	response->optionnum=0;
	char optionBuffer[2];
	char optionBuffer_1[2];

	optionBuffer[0] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0xFF00) >> 8;
	optionBuffer[1] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0x00FF) ;

	response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
	response->options[response->optionnum].length = 2;
	response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
	response->optionnum++;

	//optionBuffer_1[0] = ((uint16_t)MAX_AGE_DEFAULT & 0xFF00) >> 8;
	//optionBuffer_1[1] = ((uint16_t)MAX_AGE_DEFAULT & 0x00FF) ;

	//response->options[response->optionnum].buffer = (uint8_t *)optionBuffer_1;
	//response->options[response->optionnum].length = 2;
	//response->options[response->optionnum].number =COAP_MAX_AGE ;
	//response->optionnum++;

	response->code=COAP_CONTENT ;
	response->payload=(uint8_t *)payload;
	response->payloadlen=strlen(payload);

	sendPacket(response,Udp.remoteIP(),Udp.remotePort());
}


void coapServer::sendResponse( IPAddress ip, int port, char *payload) {


	if(request->code_()==COAP_GET){
		response->code = COAP_CONTENT;
		response->payload = (uint8_t *)payload;
		response->payloadlen = strlen(payload);
		response->optionnum = 0;

		uint8_t num;
		for(uint8_t i=0;i<=request->optionnum;i++)
		{
			if(request->options[i].number==COAP_OBSERVE){
				num=i;
				break;
			}
		}


		if(request->options[num].number==COAP_OBSERVE){


			obsstate=obsstate+1;

			response->options[response->optionnum].buffer =&obsstate;		
			response->options[response->optionnum].length = 1;
			response->options[response->optionnum].number = COAP_OBSERVE;
			response->optionnum++;
		}

		char optionBuffer2[2];
		optionBuffer2[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
		optionBuffer2[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer2;		
		response->options[response->optionnum].length = 2;
		response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
		response->optionnum++;

		sendPacket(response, ip, port);


	}else if(request->code_()==COAP_PUT){
		String str="PUT OK";
		const char *payload=str.c_str();
		response->code = COAP_CHANGED;
		response->payload=(uint8_t *)payload;
		response->payloadlen = strlen(payload);
		response->optionnum = 0;
		char optionBuffer[2];
		optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
		optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;		
		response->options[response->optionnum].length = 2;
		response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
		response->optionnum++;

		sendPacket(response, ip, port);

	}else if(request->code_()==COAP_POST){
		String str="Post changed";
		const char *payload=str.c_str();
		response->code = COAP_CHANGED;
		response->payload=(uint8_t *)payload;
		response->payloadlen = strlen(payload);
		response->optionnum = 0;
		char optionBuffer[2];
		optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
		optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;		
		response->options[response->optionnum].length = 2;
		response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
		response->optionnum++;

		sendPacket(response, ip, port);

	}

}

//add observer
void coapServer::addObserver(String url,coapPacket *request,IPAddress ip,int port){

	//uri.find(url)(request,Udp.remoteIP(),Udp.remotePort());
	uri.find(url)(request,ip,port,0);


	//storing the details of clients 
	observer[obscount].observer_token=request->token;
	observer[obscount].observer_tokenlen=request->tokenlen;
	observer[obscount].observer_clientip=ip;
	observer[obscount].observer_clientport=port;
	observer[obscount].observer_url=url;

	obscount=obscount+1;



}

//make nofification packet and send 
void coapServer::notification(char *payload)
{

	response->version=request->version;
	response->type=COAP_ACK;

	response->messageid=messid++;

	response->code = COAP_CONTENT;
	response->payload = (uint8_t *)payload;
	response->payloadlen = strlen(payload);
	response->optionnum = 0;

	obsstate=obsstate+1;

	response->options[response->optionnum].buffer =&obsstate;		
	response->options[response->optionnum].length = 1;
	response->options[response->optionnum].number = COAP_OBSERVE;
	response->optionnum++;


	char optionBuffer2[2];
	optionBuffer2[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
	optionBuffer2[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
	response->options[response->optionnum].buffer = (uint8_t *)optionBuffer2;		
	response->options[response->optionnum].length = 2;
	response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
	response->optionnum++;

	for(uint8_t i=0;i<obscount;i++){
		//send notification  
		if(observer[i].observer_url==resource[0].rt){
			response->token=observer[i].observer_token;
			response->tokenlen=observer[i].observer_tokenlen;
			sendPacket(response, observer[i].observer_clientip, observer[i].observer_clientport);

		}

	}
	if(messid==5000)
		messid=0;

}

//Checking for the change in resource 
void coapServer::sendResponse(char *payload)
{


	if(strcmp(previousPayload,payload)!=0){

		notification(payload);
	}

	previousPayload=payload;

}
