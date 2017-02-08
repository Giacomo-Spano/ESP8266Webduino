// 
// 
// 

#include "HtmlFileClass.h"

#include "Logger.h"
#include "css.h"
#include "html.h"
#include "js.h"

extern Logger logger;

void HtmlFileClass::init()
{


}

HtmlFileClass::HtmlFileClass()
{
	tag = "HtmlFileClass";
}

HtmlFileClass::~HtmlFileClass()
{
}

HtmlFileClass HtmlFile;

bool HtmlFileClass::sendFile(WiFiClient* pClient, String fileName) {
	
	logger.print(tag, "\n\t>>sendFile: " + fileName);

	sendHeader(pClient, fileName);

	/*if (fileName.equals("ESP8266.css")) {
		send(pClient, fileName, ESP8266_css);
	} else*/ if (fileName.equals("index.html")) {
		send(pClient, fileName, index_html);
	}
	else if (fileName.equals("settings.html")) {
		send(pClient, fileName, settings_html);
	}
	else if (fileName.equals("heater.html")) {
		send(pClient, fileName, heater_html);
	}
	/*else if (fileName.equals("webduino.js")) {
		send(pClient, fileName, webduino_js);
	}
	else if (fileName.equals("index.js")) {
		send(pClient, fileName, index_js);
	}
	else if (fileName.equals("settings.js")) {
		send(pClient, fileName, settings_js);
	}
	else if (fileName.equals("heater.js")) {
		send(pClient, fileName, heater_js);
	}*/
 	 



	/*if (fileName.equals("esp8266.css")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName,SP8266);
	}
	else if (fileName.equals("switch.css")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName,css_switch);
	}
	else if (fileName.equals("heater.html")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName, js_webduino);
		send(pClient, fileName, js_heater);		
		pClient->println("</script>");
		pClient->println("</head>");
		pClient->println("<body onload='load()'>");
		send(pClient, fileName, html_heater);
		send(pClient, fileName, html_popup);
	}
	else if (fileName.equals("settings.html")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName, js_webduino);
		send(pClient, fileName, js_settings);
		pClient->println("</script>");
		pClient->println("</head>");
		pClient->println("<body onload='load()'>");
		send(pClient, fileName, html_popup);
		send(pClient, fileName, html_settings);
	}
	else if (fileName.equals("index.html")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName, js_webduino);
		send(pClient, fileName, js_index);
		pClient->println("</script>");
		pClient->println("</head>");
		pClient->println("<body onload='load()'>");
		send(pClient, fileName, html_popup);
		send(pClient, fileName, html_index);
	}
	else if (fileName.equals("heater.js")) {
		send(pClient, fileName,js_heater);
	}
	else if (fileName.equals("webduino.js")) {
		sendHeader(pClient, fileName);
		send(pClient, fileName, js_webduino);
	}
	else if (fileName.equals("index.js")) {
		send(pClient, fileName, js_index);
	}*/
	else {
		String header = F("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\n\n");
		pClient->println(header);
	}
	pClient->stop();	
	logger.print(tag, "\n\t<<sendFilefile");

	return true;
}

void HtmlFileClass::sendHeader(WiFiClient* pClient, String fileName) {
	// HTTP response
	String header = "";
	if (fileName.endsWith(".css")) {
		header += F("HTTP/1.1 200 OK\r\nContent-Type: text/css\n\n");
	}
	else if (fileName.endsWith(".js")) {
		header += F("HTTP/1.1 200 OK\r\nContent-Type: text/javascript\n\n");
	} 
	else if (fileName.endsWith(".html")) {
		
		header += "HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n";
		/*header += "<!DOCTYPE html>"
			"<html>"
			"<head>"
			"<title>(titolo)</title>"
			"<link href = 'ESP8266.css' rel='stylesheet'/>"
			"<link href = 'switch.css' rel= stylesheet />"
			"<script type='text/javascript'>";*/
	}
	pClient->println(header);
};

void HtmlFileClass::send(WiFiClient* pClient, String fileName, const char * file) {

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