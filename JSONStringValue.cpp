#include "stdafx.h"
#include "JSONStringValue.h"
//#include<string>
//using namespace std;
//#include "Emulator/ESPEmulator.h"

JSONStringValue::JSONStringValue(String key, String value)
{
	type = JSONValueType_String;
	this->key = key;
	this->value = value;
}


JSONStringValue::~JSONStringValue()
{
}

String JSONStringValue::toJSONValueString()
{
	String str = String("\"") + key + String("\":\"") + value + String("\"");
	return str;
}
