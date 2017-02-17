// 
// 
// 

#include "ESPWebServer.h"


Logger ESPWebServer::logger;
String ESPWebServer::tag = "ESPWebServer";

const String ESPWebServer::jsonContentType = "application/json";
const String ESPWebServer::htmlContentType = "text/html";
const String ESPWebServer::cssContentType = "text/css";
const String ESPWebServer::icoContentType = "image / x - icon";
//const String ESPWebServer::attachmentContentType = "image / x - icon";

//char* ESPWebServer::jsonContentType = "application/json";

ESPWebServer::ESPWebServer(WiFiClient* pClient)
{
	this->pClient = pClient;
	response.init(pClient);
}

ESPWebServer::~ESPWebServer()
{
}

void ESPWebServer::init()
{
}

//ESPWebServer espWebServer;

HttpResponse ESPWebServer::getHttpResponse()
{
	return response;
}

HttpRequest ESPWebServer::getHttpRequest()
{
	logger.print(tag, "\n\t >> getHttpRequest");
	/*HttpRequest */request;
	request.method = HttpRequest::HTTP_NULL;

	char buffer[ESPWebServer::MAX_PAGE_NAME_LEN + 1]; // additional character for terminating null
	char parambuffer[ESPWebServer::MAX_PARAM_LEN];

	while (pClient->connected()) {

		if (pClient->available()) {
			// GET, POST, or HEAD
			memset(buffer, 0, sizeof(buffer)); // clear the buffer
			memset(parambuffer, 0, sizeof(parambuffer));
			if (pClient->readBytesUntil('/', buffer, MAX_PAGE_NAME_LEN)) {
				logger.print(tag, buffer);
				if (strcmp(buffer, "GET ") == 0) {
					request.method = HttpRequest::HTTP_GET;
					logger.print(tag, "\n\t method=HTTP_GET");
				}
				else if (strcmp(buffer, "POST ") == 0) {
					request.method = HttpRequest::HTTP_POST;
					logger.print(tag, "\n\t method=HTTP_POST");
				}
				else
				{
					request.method = HttpRequest::HTTP_NULL;
					logger.print(tag, "\n\t method=HTTP_NULL");
				}
				// look for the page name
				memset(buffer, 0, sizeof(buffer)); // clear the buffer
				int l;

				if (l = pClient->readBytesUntil(' ', buffer, MAX_PAGE_NAME_LEN))
				{
					//logger.print(tag, "\n\t l="+String(l));
					//logger.print(tag, "\n\t buffer=" + String(buffer));
					l = findIndex(buffer, "?");
					int i = 0;
					if (l != -1) {
						while ((l + i) < MAX_PAGE_NAME_LEN && i < MAX_PARAM_LEN) {
							parambuffer[i] = buffer[l + i];
							i++;
						}
						buffer[l] = '\0';
					}
					else {
						;
					}
					/*logger.println(tag, l);
					logger.println(tag, buffer);
					logger.println(tag, parambuffer);
					logger.println(tag, "-");*/

					request.page = String(buffer);
					request.param = String(parambuffer);

					logger.print(tag, "\n\t page=" + request.page);
					logger.print(tag, "\n\t param=" + request.param);
					
				}
			}

			if (request.method == HttpRequest::HTTP_POST) {
				request.body = getPostdata(pClient);
				logger.print(tag, "\n\t body=" + request.body);
			}
			logger.print(tag, "\n\t << getHttpRequest");
			return request;
		}
	}	
	// give the web browser time to receive the data
	//delay(20);
	//client.stop();
	logger.print(tag, "\n\t << getHttpRequest");
	return request;
}

String ESPWebServer::getPostdata(WiFiClient* pClient) {

	logger.print(tag, "\n\t >> getPostdata");

	String str = "";

	int datalen = 0;

	if (pClient->findUntil("Content-Length:", "\n\r"))
	{
		datalen = pClient->parseInt();
	}
	logger.print(tag, "\n\t Content-Length:" + datalen);

	delay(400);
	if (pClient->findUntil("\n\r", "\n\r"))
	{
		;
	}
	delay(400);
	pClient->read();

	int i = 0;
	while (i < datalen) {
		str += char(pClient->read());
		//Serial.print(data[i]); // ailitare questa riga per vedere il contenuto della post
		delay(2);
		i++;
	}

	
	logger.print(tag, "\n\t Content:" + str);
	//Serial.print("datalen ");
	//Serial.print(datalen);
	/*if (i < maxposdata)
	data[i] = '\0';*/

	logger.print(tag, "\n\t << getPostdata");
	return str;
}

int ESPWebServer::findIndex(const char* data, const char* target) {

	boolean found = false;
	int i = 0;
	while (data[i] != '\0') {
		i++;
	}
	i = 0;
	int k = 0;
	while (data[i] != '\0') {

		if (data[i] == target[0]) {
			found = true;
			k = 0;
			while (target[k] != '\0') {

				if (data[i + k] == '\0')
					return -1;
				if (data[i + k] != target[k]) {
					found = false;
					break;
				}
				k++;
			}
			if (found == true)
				return i;
		}
		i++;
	}
	return -1;
}

