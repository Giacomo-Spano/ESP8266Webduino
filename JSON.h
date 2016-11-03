#pragma once

#include <Arduino.h>
//#include <WString.h>

class JSON
{
public:
	JSON(String json);
	~JSON();

	String jsonGetInt(String key);
	String jsonGetString(String key);

private:
	String json;
};

