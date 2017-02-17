// 
// 
// 

#include "HttpResponse.h"
#include "ESPWebServer.h"
#include "html.h"
#include "FS.h"

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

void HttpResponse::send(const String res, String contentType, String body)
{
	logger.print(tag, "\n\t >>send");

	sendHeader(res, contentType);
	//String data;
	pClient->print(body);
	pClient->println("");
	pClient->stop();

	logger.print(tag, "\n\t <<send");
	//return data;
}

void HttpResponse::setContentDisposition(String fileName, int size)
{
	contentDisposition = "Content-Disposition: attachment; filename = \"" + fileName + " \r\n";
	contentDisposition += "Content-Length: " + String(size) + " \r\n";
}

void HttpResponse::sendHeader(const String res, String contentType)
{
	logger.print(tag, "\n\t --sendHeader");

	String data;
	data += "";
	//data += "HTTP/1.0 200 OK\r\nContent-Type: application/json; ";
	data += "HTTP/1.1 " + String(res) + " \r\n";
	//data += "Content-Type: application/json;";
	data += "Content-Type: " + contentType + "; \r\n";
	if (contentDisposition.equals("") == false) {// Content-Disposition: attachment; filename="cool.html"
												// Content - Length: 22
		data += contentDisposition;
	}
	data += "Pragma: no-cache\r\n\r\n";

	pClient->print(data);

	//logger.print(tag, "\n\t <<sendHeader" + data);
	//return data;
}

void HttpResponse::sendVirtualFile(const String res, String contentType, String fileName) {

	logger.print(tag, "\n\t>>sendFile: " + fileName);

	if (fileName.equals("index.html")) {
		sendHeader(res, contentType);
		sendData(fileName, index_html);
	}
	else if (fileName.equals("settings.html")) {
		sendHeader(res, contentType);
		sendData(fileName, settings_html);
	}
	else if (fileName.equals("heater.html")) {
		sendHeader(res, contentType);
		sendData(fileName, heater_html);
	}
	else if (fileName.equals("temperature.html")) {
		sendHeader(res, contentType);
		sendData(fileName, temperature_html);
	}
	else if (sendFile(fileName)) {
		logger.print(tag, "\n\t file: " + fileName + " sent");
	}
	else {
		String header = F("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\n\n");
		pClient->println(header);
	}

	pClient->stop();

	logger.print(tag, "\n\t<<sendFilefile");

}

void HttpResponse::sendData(String fileName, const char * file) {

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

bool HttpResponse::sendFile(String fileName) {

	logger.print(tag, "\n\t >>sendFile: " + fileName);

	File f = SPIFFS.open("/" + fileName, "r");
	if (!f) {
		logger.print(tag, "File /" + fileName + " doesn't exist");

		String data = "";
		data += "<html><body> File /" + fileName + " doesn't exist </body></html>";

		send(HTTPRESUL_NotFound, ESPWebServer::htmlContentType, data);
		return false;
	}

	int size = f.size();
	
	if (fileName.endsWith(".html"))
		sendHeader(HTTPRESULT_OK, ESPWebServer::htmlContentType);
	else if (fileName.endsWith(".css"))
		sendHeader(HTTPRESULT_OK, ESPWebServer::htmlContentType);
	else if (fileName.endsWith(".ico"))
		sendHeader(HTTPRESULT_OK, ESPWebServer::icoContentType);
	else if (fileName.endsWith(".txt")) {
		setContentDisposition("log.txt",size);
		sendHeader(HTTPRESULT_OK, ESPWebServer::htmlContentType);
	}

	// we could open the file
	
	String line = "";
	int sent = 0;
	while (f.available()) {
		//Lets read line by line from the file
		line += f.readStringUntil('\n');

		if (line.length() > 10000) {
			sent += line.length();
			String txt = "sent " + String(sent) + " of" + String(size);
			logger.print(tag, txt);
			pClient->println(line);
			//pClient->print("\r\n");
			line = "";
			//delay(50);
		}
	}
	f.close();

	//logger.print(tag, line);
	pClient->print(line);
	pClient->println("");
	pClient->stop();

	//logger.print(tag, line);
	
	
	/*if (fileName.endsWith(".html"))
		send(HTTPRESULT_OK, ESPWebServer::htmlContentType, line);
	else if (fileName.endsWith(".css"))
			send(HTTPRESULT_OK, ESPWebServer::htmlContentType, line);
	else if (fileName.endsWith(".ico"))
		send(HTTPRESULT_OK, ESPWebServer::icoContentType, line);*/
	
	//pClient->print(str);
	//pClient->stop();

	logger.print(tag, "\n\t<<sendFilefile");

	return true;
}




