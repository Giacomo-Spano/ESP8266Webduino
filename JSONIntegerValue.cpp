#include "stdafx.h"
#include "JSONIntegerValue.h"
//#include "Emulator\ESPEmulator.h"


JSONIntegerValue::JSONIntegerValue(String key, int value)
{
	type = JSONValueType_Integer;
	this->key = key;
	this->value = value;
}


JSONIntegerValue::~JSONIntegerValue()
{
}

String JSONIntegerValue::toJSONValueString()
{
	String str = String("\"") + key + String("\":");
	//str += value;
	str = str + value;
	return str;
}
