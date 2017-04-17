#include "stdafx.h"
#include "JSONArrayValue.h"


JSONArrayValue::JSONArrayValue(String key, String value)
{
	type = JSONValueType_JSONArray;
	this->key = key;
	this->value = value;
}


JSONArrayValue::~JSONArrayValue()
{
}

String JSONArrayValue::toJSONValueString()
{
	//return "";
	String str = String("\"") + key + String("\":") + value + String("");
	return str;
}
