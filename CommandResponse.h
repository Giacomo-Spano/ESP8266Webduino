#ifndef _COMMANDRESPONSE_h
#define _COMMANDRESPONSE_h

#include <Arduino.h>
#include "Logger.h"

class CommandResponse
{
private:
	static String tag;
	static Logger logger;

public:
	
	CommandResponse();
	~CommandResponse();

	String result;
	String error;
	String uuid;

	virtual String getJSONFields();
	virtual String getJSON();
};

#endif
