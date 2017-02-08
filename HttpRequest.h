// HttpRequest.h

#ifndef _HTTPREQUEST_h
#define _HTTPREQUEST_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include "Logger.h"


class HttpRequest
{
public:
	typedef enum {
		HTTP_NULL = 0,
		HTTP_GET = 1,
		HTTP_POST = 2,
		HTTP_DELETE = 3,
		HTTP_PUT = 4,
		HTTP_PATCH = 5,
		HTTP_HEAD = 6,
		HTTP_OPTIONS = 7,
		HTTP_ANY = 8		
	} HttpRequestMethod;

 protected:
	 

 public:
	 HttpRequest();
	 ~HttpRequest();
	void init();
	HttpRequestMethod method;
	String page;
	String param;
	String body;

private:
	static String tag;
	static Logger logger;
	
};

extern HttpRequest httpRequest;

#endif

