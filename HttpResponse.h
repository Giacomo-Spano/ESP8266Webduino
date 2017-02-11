// HttpResponse.h

#ifndef _HTTPRESPONSE_h
#define _HTTPRESPONSE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"
#include "WiFiClient.h"

class HttpResponse
{	
public:
	static const String HTTPRESULT_OK;// = "200 OK",
	static const String HTTPRESUL_NotFound; // 404

	HttpResponse();
	~HttpResponse();
	void init(WiFiClient* pClient);
	String send(const String res, String contentType, String body);
	bool sendFile(const String res, String contentType, String fileName);
	
private:
	static String tag;
	static Logger logger;
	WiFiClient* pClient;
	void sendData(WiFiClient* pClient, String fileName, const char * file);
	String sendHeader(const String res, String contentType);

};

extern HttpResponse httpResponse;



#endif

