
#include "coap.h"

//creating instance for required class
WiFiUDP udp;
CoapUri uri;
CoapResource resource[MAX_OPTION_NUM];
CoapPacket *request=new CoapPacket();
CoapPacket *response=new CoapPacket();

//counter for maintaining resource count
static uint8_t rcount=0;

CoapUri::CoapUri() {
	for (int i = 0; i < MAX_CALLBACK; i++) {
		u[i] = "";
		c[i] = NULL;
	}
}

//adding resources 
void CoapUri::add(callback call, String url,CoapResource resource[]) {

	for (int i = 0; i < MAX_CALLBACK; i++)
		if (c[i] != NULL && u[i].equals(url)) {
			c[i] = call;
			rcount++;

			resource[i].rt=url;
			resource[i].ct=0;

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
callback CoapUri::find(String url) {
	for (int i = 0; i < MAX_CALLBACK; i++) if (c[i] != NULL && u[i].equals(url)) return c[i];
	return NULL;
}

void Coap::server(callback c, String url) {
	uri.add(c, url,resource); 
}

Coap::Coap() {
}


bool Coap::start() {
	this->start(COAP_DEFAULT_PORT);
	return true;
}

bool Coap::start(int port) {
	udp.begin(port);
	return true;
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

//parse option
int CoapPacket::parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
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

bool Coap::loop() {

	uint8_t buffer[BUF_MAX_SIZE];
	int32_t packetlen = udp.parsePacket();

	if (packetlen > 0) {

		packetlen = udp.read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);

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
		if(request->code_()==COAP_EMPTY)
		{

			response->version=request->version;
			response->type=COAP_RESET;
			response->code=COAP_EMPTY_MESSAGE;
			response->messageid=request->messageid;
			response->token=request->token;
			response->payload=NULL;
			response->payloadlen=0;
			sendPacket(response,udp.remoteIP(),udp.remotePort());


		}
		else if(request->code_()==COAP_GET){

			if(request->type_()== COAP_CON){

				response->version=request->version;
				response->type=COAP_ACK;
				response->tokenlen=request->tokenlen;
				response->messageid=request->messageid;
				response->token=request->token;
			}
			else if (request->type_()==COAP_NONCON){
				response->version=request->version;
				response->type=COAP_ACK;
				response->tokenlen=request->tokenlen;
				response->messageid=request->messageid;
				response->token=request->token;
			}


			if(url==String(".well-known/core")){

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
}


void CoapPacket::bufferToPacket(uint8_t buffer[],int32_t packetlen){

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
		packetlen =udp.parsePacket();

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

uint16_t Coap::sendPacket(CoapPacket *packet, IPAddress ip, int port) {

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

	udp.beginPacket(ip, port);
	udp.write(buffer, packetSize);
	udp.endPacket();
}

void Coap::resourceDiscovery(CoapPacket *response,IPAddress ip, int port,CoapResource resource[])
{

	String str_res;

	for(int i=0;i<rcount;i++)
	{
		str_res +="</";
		str_res +=resource[i].rt;
		str_res +=">;";
		str_res +="rt=";
		str_res +="\"";
		str_res +=resource[i].rt;
		str_res +="\"";
		str_res +=";";
		str_res +="ct=";
		str_res +=resource[i].ct;
		str_res +=",";
	}

	const char *payload=str_res.c_str();

	response->optionnum=0;
	char optionBuffer[2];
	optionBuffer[0] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0xFF00) >> 8;
	optionBuffer[1] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0x00FF) ;

	response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;
	response->options[response->optionnum].length = 2;
	response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
	response->optionnum++;

	response->code=COAP_CONTENT ;
	response->payload=(uint8_t *)payload;
	response->payloadlen=strlen(payload);

	sendPacket(response,udp.remoteIP(),udp.remotePort());
}



void Coap::sendResponse(CoapPacket *packet, IPAddress ip, int port, char *payload) {


	response->code = COAP_CONTENT;
	response->payload = (uint8_t *)payload;
	response->payloadlen = strlen(payload);
	response->optionnum = 0;
	char optionBuffer[2];
	optionBuffer[0] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0xFF00) >> 8;
	optionBuffer[1] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT& 0x00FF) ;
	response->options[response->optionnum].buffer = (uint8_t *)optionBuffer;		
	response->options[response->optionnum].length = 2;
	response->options[response->optionnum].number = COAP_CONTENT_FORMAT;
	response->optionnum++;

	sendPacket(response, ip, port);


}




