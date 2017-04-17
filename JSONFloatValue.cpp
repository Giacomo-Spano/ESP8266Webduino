#include "stdafx.h"
#include "JSONFloatValue.h"


JSONFloatValue::JSONFloatValue(String key, float value)
{
	type = JSONValueType_Float;
	this->key = key;
	this->value = value;
}

JSONFloatValue::~JSONFloatValue()
{
}

String JSONFloatValue::toJSONValueString()
{
	String str = String("\"") + key + String("\":");
	str = str + value;
	return str;
}
