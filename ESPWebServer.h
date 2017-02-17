// ESPWebServer.h

#ifndef _ESPWEBSERVER_h
#define _ESPWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "WiFiClient.h"


class ESPWebServer
{
public:
	//static const char* jsonContentType;// = "application/json",
	static const String jsonContentType;// = "application/json";
	static const String htmlContentType;// = "application/json";
	static const String cssContentType;// = "text/css;";
	static const String icoContentType;// = "image / x - icon;";

		

	ESPWebServer(WiFiClient* pClient);
	~ESPWebServer();
	void init();

	HttpRequest getHttpRequest();
	HttpResponse getHttpResponse();

private:
	static String tag;
	static Logger logger;
	WiFiClient* pClient;
	HttpRequest request;
	HttpResponse response;

	static const int MAX_PAGE_NAME_LEN = 32;
	static const int MAX_PARAM_LEN = 32;
	int findIndex(const char* data, const char* target);
	String getPostdata(WiFiClient* pClient);
};

extern ESPWebServer espWebServer;

#endif

