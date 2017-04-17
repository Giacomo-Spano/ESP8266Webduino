#pragma once
#include "JSONValue.h"

class JSONArrayValue :
	public JSONValue
{
public:
	JSONArrayValue(String key, String value);
	~JSONArrayValue();

	virtual String toJSONValueString();

	String value;
};

