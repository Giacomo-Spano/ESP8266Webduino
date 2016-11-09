#pragma once

#include <Arduino.h>
//#include <WString.h>

class JSON
{
public:
	JSON(String json);
	~JSON();

	
	int jsonGetInt(String key);
	long jsonGetLong(String key);
	String jsonGetString(String key);

private:
	String json;
	String jsonGetValue(String key);
};

