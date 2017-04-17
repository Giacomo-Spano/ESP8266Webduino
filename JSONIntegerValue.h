#pragma once
#include "JSONValue.h"

class JSONIntegerValue :
	public JSONValue
{
public:
	JSONIntegerValue(String key, int value);
	~JSONIntegerValue();

	virtual String toJSONValueString();

	int value;
};

