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
	void send(const String res, String contentType, bool body);
	void sendVirtualFile(const String res, String contentType, String fileName);
	
	
private:
	static String tag;
	static Logger logger;
	WiFiClient* pClient;
	String contentDisposition = "";
	void sendData(String fileName, const char * file);
	void sendHeader(const String res, String contentType);
	void setContentDisposition(String fileName, int size);
	bool sendFile(String fileName);

};

extern HttpResponse httpResponse;



#endif

