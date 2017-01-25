#pragma once

#include <Arduino.h>
#include "Logger.h"
//#include <WString.h>

class JSON
{
public:
	JSON(String json);
	~JSON();

	
	int jsonGetInt(String key);
	bool jsonGetBool(String key);
	long jsonGetLong(String key);
	String jsonGetString(String key);
	float jsonGetFloat(String key);
	bool has(String key);

private:
	String tag;
	String jsonString;
	String getRightOfKey(String key);
	String getNum(String key);
	bool getBool(String key);
	String getString(String key);
};

