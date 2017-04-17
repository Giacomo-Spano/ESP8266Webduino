#include "stdafx.h"
#include "JSONBoolValue.h"


JSONBoolValue::JSONBoolValue(String key, bool value)
{
	type = JSONValueType_Boolean;
	this->key = key;
	this->value = value;
}


JSONBoolValue::~JSONBoolValue()
{
}

String JSONBoolValue::toJSONValueString()
{
	String str = String("\"") + key + String("\":");
	if (value)
		str = str + "true";
	else
		str = str + "false";
	return str;
}

