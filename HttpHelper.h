#pragma once
#include "Arduino.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#else
#include "WiFi.h"
#endif




#include "Logger.h"



class HttpHelper
{
private:
	static const int MAX_PAGE_NAME_LEN = 32;
	static const int MAX_PARAM_LEN = 32;//12;
	
	//Logger logger;

	int findIndex(const char* data, const char* target);

public:
String tag;

	HttpHelper();
	~HttpHelper();

	bool post(String host, int port, char* path, char* param, int len, String *result);
	bool post(char* host, int port, char* path, char* param, int len, String *result);
	bool post(String host, int port, String path, String param, String *result);
	boolean getNextPage(WiFiClient* pClient, WiFiServer* server, String* page, String* param);
	bool downloadfile(String filename, String host, int port, String path, String param, String *result);
};

