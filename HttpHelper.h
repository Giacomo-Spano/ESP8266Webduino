#pragma once

#include <ESP8266WiFi.h>


class HttpHelper
{
private:
	
public:


	HttpHelper();
	~HttpHelper();

	bool post(char* host, int port, char* path, char* param, int len, String *result);
};

