#include "coap.h"


WiFiUDP udp;
CoapUri uri;
CoapResource resource[MAX_OPTION_NUM];
CoapPacket *request=new CoapPacket();
CoapPacket *response=new CoapPacket();

static uint8_t rcount=0;

CoapUri::CoapUri() {
	for (int i = 0; i < MAX_CALLBACK; i++) {
		u[i] = "";
		c[i] = NULL;
	}
}

void CoapUri::add(callback call, String url,CoapResource resource[]) {

	Serial.println("resource add loop");

	for (int i = 0; i < MAX_CALLBACK; i++)
		if (c[i] != NULL && u[i].equals(url)) {
			Serial.println("resource adding second tym");
			c[i] = call;
			rcount++;
			Serial.print("rcount=");
			Serial.println(rcount);
			resource[i].rt=url;
			resource[i].ct=0;
			Serial.print("resource url");
			Serial.println(resource[i].rt);
			return ;
		

		}
	for (int i = 0; i < MAX_CALLBACK; i++) 
		if (c[i] == NULL) {
			Serial.println("resource adding first tym");
			c[i] = call;
			u[i] = url;
			rcount++;
			Serial.print("rcount=");
			Serial.println(rcount);
			resource[i].rt=url;
			resource[i].ct=0;
			Serial.print("resource url");
			Serial.println(resource[i].rt);
			return;
		}
Serial.print("rcount=");
Serial.println(rcount);
}

callback CoapUri::find(String url) {
	for (int i = 0; i < MAX_CALLBACK; i++) if (c[i] != NULL && u[i].equals(url)) return c[i];
	return NULL;
}

Coap::Coap(

	  ) {
Serial.println("Default coap constructor");

}

bool Coap::start() {
	Serial.println("start loop");

	this->start(COAP_DEFAULT_PORT);
	return true;
}

bool Coap::start(int port) {
Serial.println("start loop....");

	udp.begin(port);
	return true;
}

void Coap::server(callback c, String url) {
        Serial.println("server loop");
	uri.add(c, url,resource); 
}

CoapPacket::CoapPacket()
  {
  }

uint8_t CoapPacket::version_(){
return version;
}

uint8_t CoapPacket::type_(){
return type;
}

uint8_t CoapPacket::code_(){
return code;
}

uint16_t CoapPacket::messageid_(){
return messageid;
}

uint8_t * CoapPacket::token_(){
return token;
}




int CoapPacket::parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {

	Serial.println("parseoption loop");

	uint8_t *p = *buf;
	uint8_t headlen = 1;
	uint16_t len, delta;

	if (buflen < headlen) return -1;

	delta = (p[0] & 0xF0) >> 4;
	len = p[0] & 0x0F;
	
	Serial.print("delta");
	Serial.println(delta);
	
	Serial.print("len");
	Serial.println(len);

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

bool Coap::loop() {

Serial.println("main loop starts");

	uint8_t buffer[BUF_MAX_SIZE];
	int32_t packetlen = udp.parsePacket();



	if (packetlen > 0) {

		packetlen = udp.read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);

		
//CoapPacket *request;
		request->buffer_to_packet(buffer,packetlen);







		String url = "";
			for (int i = 0; i < request->optionnum; i++) {
				if (request->options[i].number == COAP_URI_PATH && request->options[i].length > 0) {
				char urlname[request->options[i].length + 1];
				memcpy(urlname,request->options[i].buffer,request->options[i].length);
				urlname[request->options[i].length] = NULL;
				if(url.length() > 0)
					url += "/";
				url += urlname;
Serial.print("url=");
Serial.println(url);

				}
			}

	
//CoapPacket *response;





if(request->code_()==COAP_GET || request->code_()==COAP_POST ||request->code_()==COAP_PUT || request->code_()==COAP_DELETE){


Serial.println("switch case");
switch(request->type_()){
case COAP_CON:
response->version=request->version;
response->type=COAP_ACK;
response->tokenlen=request->tokenlen;
response->messageid=request->messageid;
response->token=request->token;
break;
case COAP_NONCON:
response->version=request->version;
response->type=COAP_ACK;
response->tokenlen=request->tokenlen;
response->messageid=request->messageid;
response->token=request->token;

break;
}

}


Serial.print("request->options[0].number=");
Serial.println(request->options[0].number);






if(request->code_()==COAP_EMPTY)
{
Serial.println("ping");

response->type=COAP_RESET;
Serial.print("response->type=");
Serial.print(response->type);

response->code=COAP_NULL;
Serial.print("response->code=");
Serial.print(response->code);

response->messageid=request->messageid;
response->token=request->token;

response->payload=NULL;
response->payloadlen=0;

sendPacket(response,udp.remoteIP(),udp.remotePort());


}

else if(url==String(".well-known/core")){
Serial.print("resource discovery");
resourceDiscovery(response,udp.remoteIP(),udp.remotePort(),resource);

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

sendPacket(response,udp.remoteIP(),udp.remotePort());	

}else
{

uri.find(url)(response,udp.remoteIP(),udp.remotePort());

}



 



		
}
}





void CoapPacket::buffer_to_packet(uint8_t buffer[],int32_t packetlen){

Serial.println("buffer to packet");

        version=(buffer[0] & 0xC0)>>6;
	type= (buffer[0] & 0x30) >> 4;
	tokenlen = buffer[0] & 0x0F;
	code = buffer[1];
	messageid = 0xFF00 & (buffer[2] << 8);
	messageid |= 0x00FF & buffer[3];

	Serial.print("type=");
	Serial.println(type);
	Serial.print("tokenlen=");
	Serial.println(tokenlen);
	Serial.print("code=");
	Serial.println(code);
	Serial.print("messageid=");
	Serial.println(messageid);

	if (tokenlen == 0)  token = NULL;
	else if (tokenlen <= 8) 
	{       token=new uint8_t(tokenlen);
		memset(token,0,tokenlen);
		for(int i=0;i<tokenlen ;i++)
		{
			token[i]=buffer[4+i];
		}
		for(int i=0;i<tokenlen;i++)
		{
			Serial.print(token[i]);
			Serial.print("\n");
		}

	}
	else {
		packetlen =udp.parsePacket();
		//continue;
	}

	// parse packet options/payload
	if (COAP_HEADER_SIZE +tokenlen < packetlen) {
		
		Serial.println("parse options and payload");

		int optionIndex = 0;
		uint16_t delta = 0;
		uint8_t *end = buffer + packetlen;
		uint8_t *p = buffer + COAP_HEADER_SIZE+tokenlen;

		while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {          
			Serial.println("options while loop");
			options[optionIndex];
			if (0 ==parseOption(&options[optionIndex], &delta, &p, end-p))
			
				optionIndex++;
Serial.print("optionIndex=");
Serial.println(optionIndex);

		}
		optionnum = optionIndex;
		Serial.print("optionnum=");
		Serial.println(optionnum);

		if (p+1 < end && *p == 0xFF) {
			payload = p+1;
			payloadlen = end-(p+1);
		} else {

			payload = NULL;
			payloadlen= 0;
		}
	}






}

uint16_t Coap::sendPacket(CoapPacket *packet, IPAddress ip, int port) {

	Serial.print("packet sendinggggg");
	Serial.print("\n");	

	uint8_t buffer[BUF_MAX_SIZE];
	uint8_t *p = buffer;
	uint16_t running_delta = 0;
	uint16_t packetSize = 0;

	Serial.print("packet header");
	Serial.print("\n");

	// make coap packet base header
	*p = 0x01 << 6;
	*p |= (packet->type & 0x03) << 4;
	*p++ |= (packet->tokenlen & 0x0F);
	*p++ = packet->code;
	*p++ = (packet->messageid >> 8);
Serial.print("packet->messageid=");
Serial.println(packet->messageid);

	*p++ = (packet->messageid & 0xFF);
	p = buffer + COAP_HEADER_SIZE;
	packetSize += 4;



	// make token
	if (packet->token != NULL && packet->tokenlen <=8) {
		Serial.print("packet token");
		Serial.print("\n");
		memcpy(p, packet->token, packet->tokenlen);
		p += packet->tokenlen;
		packetSize += packet->tokenlen;
	}

	// make option header
	for (int i = 0; i < packet->optionnum; i++)  {
		Serial.print("packet option");
		Serial.print("\n");
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
		Serial.println("packet payload");
		if ((packetSize + 1 + packet->payloadlen) >= BUF_MAX_SIZE) {
			return 0;
		}
		*p++ = 0xFF;
		memcpy(p, packet->payload, packet->payloadlen);
		packetSize += 1 + packet->payloadlen;
	}

	udp.beginPacket(ip, port);
	udp.write(buffer, packetSize);
	udp.endPacket();
}

void Coap::resourceDiscovery(CoapPacket *response,IPAddress ip, int port,CoapResource resource[])
	{
		Serial.print("resource discovery loop");
		Serial.print("\n");
		//Serial.print("rcount=");
		//Serial.print(rcount);
		//Serial.print("\n");
		String strres;

		for(int i=0;i<rcount;i++)
		{
			strres +="</";
			strres +=resource[i].rt;
			strres +=">;";
			strres +="rt=";
			strres +="\"";
			strres +=resource[i].rt;
			strres +="\"";
			strres +=";";
			strres +="ct=";
			strres +=resource[i].ct;
			strres +=",";
		}
		Serial.print("strres=");
		Serial.print(strres);
		Serial.print("\n");

		const char *payload=strres.c_str();

		Serial.print("payload=");
		Serial.print(payload);
		Serial.print("\n");





response->optionnum=0;


char optionBuffer[2];
/*
optionBuffer[0] = ((uint16_t) MAX_AGE_DEFAULT & 0xFF00) >> 8;
		optionBuffer[1] = ((uint16_t) MAX_AGE_DEFAULT & 0x00FF) ;
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
		response->options[response->optionnum].length = 4;
		response->options[response->optionnum].number = COAP_MAX_AGE ;
		response->optionnum++;
*/





		optionBuffer[0] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0xFF00) >> 8;
Serial.print("optionBuffer[0]=");
Serial.println(optionBuffer[0]);

		optionBuffer[1] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0x00FF) ;
Serial.print("optionBuffer[1]=");
Serial.println(optionBuffer[1]);
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
		response->options[response->optionnum].length = 2;
		response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
		response->optionnum++;
Serial.print("response->optionnum=");
Serial.println(response->optionnum);

Serial.print("optionBuffer=");
Serial.println(optionBuffer);

optionBuffer[0] = ((uint16_t) MAX_AGE_DEFAULT & 0xFF00) >> 8;
Serial.print("optionBuffer[0]=");
Serial.println(optionBuffer[0]);
		optionBuffer[1] = ((uint16_t)  MAX_AGE_DEFAULT & 0x00FF) ;
Serial.print("optionBuffer[1]=");
Serial.println(optionBuffer[1]);

Serial.print("optionBuffer=");
Serial.println(optionBuffer);

		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
		response->options[response->optionnum].length = 2;
		response->options[response->optionnum].number = COAP_MAX_AGE ;
		response->optionnum++;








	
response->code=COAP_CONTENT ;
response->payload=(uint8_t *)payload;
response->payloadlen=strlen(payload);


sendPacket(response,udp.remoteIP(),udp.remotePort());







	}



	uint16_t Coap::sendResponse(CoapPacket *packet, IPAddress ip, int port, char *payload) {

		this->sendResponse(ip, port, packet->messageid,packet->type, payload, strlen(payload), COAP_CONTENT, COAP_TEXT_PLAIN, packet->token, packet->tokenlen);
	}
	

	uint16_t Coap::sendResponse(IPAddress ip, int port, uint16_t messageid,uint8_t type, char *payload, int payloadlen,
			COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE contentype, uint8_t *token, uint8_t tokenlen) {

		Serial.print("Sendresponse");
		Serial.print("\n");


	
		response->code = code;
		
		response->payload = (uint8_t *)payload;
		response->payloadlen = payloadlen;
		
		response->optionnum = 0;
		// if more options?
		char optionBuffer[2];
		optionBuffer[0] = ((uint16_t)contentype & 0xFF00) >> 8;
		optionBuffer[1] = ((uint16_t)contentype & 0x00FF) ;
		response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;		response->options[response->optionnum].length = 2;


		response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
		response->optionnum++;
                

		return this->sendPacket(response, ip, port);
	}





