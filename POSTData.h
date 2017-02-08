// POSTData.h

#ifndef _POSTDATA_h
#define _POSTDATA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"

class POSTData
{
public:
	POSTData(String data);
	~POSTData();
	bool has(String key);
	String getString(String key);
	
private:
	static String tag;
	static Logger logger;
	String dataString;
	String getRightOfKey(String key);
};

#endif

