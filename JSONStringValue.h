#pragma once
#include "JSONValue.h"

class JSONStringValue :
	public JSONValue
{
public:
	JSONStringValue(String key, String value);
	~JSONStringValue();

	virtual String toJSONValueString();
	
	String value;
};

