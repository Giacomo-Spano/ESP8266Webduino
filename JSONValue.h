#pragma once
#include "ObjectClass.h"

enum JSONValueType { JSONValueType_Void, JSONValueType_String, JSONValueType_Integer, JSONValueType_Float, JSONValueType_Boolean, JSONValueType_JSONArray };

class JSONValue :
	public ObjectClass
{
public:
	JSONValue();
	~JSONValue();

	String key;
	JSONValueType type;

	virtual String toJSONValueString();

protected:
	

};

