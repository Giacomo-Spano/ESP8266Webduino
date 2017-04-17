#pragma once
#include "JSONValue.h"
class JSONBoolValue :
	public JSONValue
{
public:
	JSONBoolValue(String key, bool value);
	~JSONBoolValue();

	virtual String toJSONValueString();

	bool value;
};

