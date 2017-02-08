// HtmlFile.h

#ifndef _HTMLFILE_h
#define _HTMLFILE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <WiFiClient.h>

class HtmlFileClass
{
 protected:
	 String tag;
	 
	 void send(WiFiClient* pClient, String fileName, const char * file);

 public:
	 HtmlFileClass();
	 ~HtmlFileClass();
	void init();
	bool sendFile(WiFiClient* pClient, String fileName);
	void sendHeader(WiFiClient* pClient, String fileName);
};

extern HtmlFileClass HtmlFile;

#endif

