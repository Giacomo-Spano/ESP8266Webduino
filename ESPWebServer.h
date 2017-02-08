// ESPWebServer.h

#ifndef _ESPWEBSERVER_h
#define _ESPWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "HttpRequest.h"
#include "WiFiClient.h"


class ESPWebServer
{
public:
	ESPWebServer();
	~ESPWebServer();
	void init();
	HttpRequest getHttpRequest(WiFiClient* pClient);

private:
	static String tag;
	static Logger logger;
	static const int MAX_PAGE_NAME_LEN = 32;
	static const int MAX_PARAM_LEN = 32;
	int findIndex(const char* data, const char* target);
	String getPostdata(WiFiClient* pClient);
};

extern ESPWebServer espWebServer;

#endif

