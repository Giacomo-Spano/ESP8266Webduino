#pragma once
#include "JSONValue.h"
class JSONFloatValue :
	public JSONValue
{
public:
	JSONFloatValue(String key, float value);
	~JSONFloatValue();

	virtual String toJSONValueString();

	float value;
};

