#pragma once

#include <Arduino.h>
#include "Logger.h"

class JSON
{
public:
	JSON(String json);
	~JSON();	
	int jsonGetInt(String key);
	bool jsonGetBool(String key);
	long jsonGetLong(String key);
	String jsonGetString(String key);
	String jsonGetArrayString(String key);
	float jsonGetFloat(String key);
	bool has(String key);

private:
	static String tag;
	static Logger logger;
	String jsonString;
	String getRightOfKey(String key);
	String getNum(String key);
	bool getBool(String key);
	String getString(String key);
};

