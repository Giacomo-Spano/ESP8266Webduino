// JSONArray.h

#ifndef _JSONARRAY_h
#define _JSONARRAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"

class JSONArray
{
public:
	JSONArray(String json);
	~JSONArray();
	/*int jsonGetInt(String key);
	bool jsonGetBool(String key);
	long jsonGetLong(String key);
	String jsonGetString(String key);
	float jsonGetFloat(String key);
	bool has(String key);*/
	String getFirst();
	String getNext();

private:
	static String tag;
	static Logger logger;
	String jsonArrayString;
	int start;
	int end;
	/*String getRightOfKey(String key);
	String getNum(String key);
	bool getBool(String key);
	String getString(String key);*/
};



#endif

