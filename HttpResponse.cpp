// 
// 
// 

#include "HttpResponse.h"
#include "html.h"

Logger HttpResponse::logger;
String HttpResponse::tag = "HttpResponse";
const String HttpResponse::HTTPRESULT_OK = "200 OK";
const String HttpResponse::HTTPRESUL_NotFound = "404 Not Found";

HttpResponse::HttpResponse()
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::init(WiFiClient* pClient)
{
	this->pClient = pClient;
}

HttpResponse httpResponse;

String HttpResponse::send(const String res,String contentType, String body)
{
	logger.print(tag, "\n\t >>send");

	sendHeader(res, contentType);
	String data;
	pClient->print(body);
	pClient->println("");
	pClient->stop();

	logger.print(tag, "\n\t <<send" + data);
	return data;
}

String HttpResponse::sendHeader(const String res, String contentType)
{
	logger.print(tag, "\n\t >>sendHeader");

	String data;
	data += "";
	//data += "HTTP/1.0 200 OK\r\nContent-Type: application/json; ";
	data += "HTTP/1.0 " + String(res) + " \r\n";
	//data += "Content-Type: application/json;";
	data += "Content-Type: " + contentType + ";";
	data += " \r\nPragma: no-cache\r\n\r\n";

	pClient->print(data);

	logger.print(tag, "\n\t <<sendHeader" + data);
	return data;
}


bool HttpResponse::sendFile(const String res, String contentType, String fileName) {

	logger.print(tag, "\n\t>>sendFile: " + fileName);
		
	if (fileName.equals("index.html")) {
		sendHeader(res, contentType);
		sendData(pClient, fileName, index_html);
	}
	else if (fileName.equals("settings.html")) {
		sendHeader(res, contentType);
		sendData(pClient, fileName, settings_html);
	}
	else if (fileName.equals("heater.html")) {
		sendHeader(res, contentType);
		sendData(pClient, fileName, heater_html);
	}
	
	else {
		String header = F("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\n\n");
		pClient->println(header);
	}

	pClient->stop();
	
	logger.print(tag, "\n\t<<sendFilefile");
	return true;
}

void HttpResponse::sendData(WiFiClient* pClient, String fileName, const char * file) {

	//
	int len = strlen_P(file);
	int i = 0, k = 0;
	while (i < len) {
		logger.print(tag, "\n\t ...sending file: " + fileName + String(i) + " bytes of" + String(len));
		String str = "";
		for (k = 0; k < 500; k++)
		{
			if (i + k >= len) {
				break;
			}
			char s = pgm_read_byte_near(file + i + k);
			str += s;
		}
		i += k;
		pClient->print(str);
		delay(50);
	}
}



