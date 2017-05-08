#include "stdafx.h"
#include "JSONObject.h"
#include "Tokener.h"
#include "JSONStringValue.h"
#include "JSONIntegerValue.h"
#include "JSONFloatValue.h"
#include "JSONBoolValue.h"
#include "JSONArrayValue.h"
#include "JSONArrayObject.h"

Logger JSONObject::logger;
String JSONObject::tag = "Shield";

JSONObject::JSONObject(String json)
{
	parse(json);
}

JSONObject::JSONObject()
{
	
}

JSONObject::~JSONObject()
{
	map.clearAll();
}

/*JSONObject & JSONObject::operator=(const JSONObject & rhs)
{
	// TODO: insert return statement here
	ObjectClass* p = rhs.map.getFirst();
	int i = 0;
	while (p != nullptr) {
		
		p = p->next;
	}

	return *this;
}*/

// Take a const-reference to the right-hand side of the assignment.
// Return a non-const reference to the left-hand side.
/*JSONObject& JSONObject::operator=(const JSONObject &rhs) {
	// Do the assignment operation!

		return *this;  // Return a reference to myself.
}*/

String JSONObject::toString()
{
	String res = "{";
	for (int i = 0; i < map.length(); i++) {
		if (i > 0)
			res = res + ",";
		JSONValue* jsonValue = (JSONValue*)map.get(i);
		String str = jsonValue->toJSONValueString();
		res = res + str;
	}
	res = res + "}";
	return res;
}

String JSONObject::toFormattedString()
{
	/*String res = "\n{";

	res += "\n";
	for (int i = 0; i < map.length(); i++) {
		if (i > 0)
			res = res + ",\n";

		JSONValue* jsonValue = (JSONValue*)map.get(i);
		res += "\"" + jsonValue->key + "\":";				
		String str = jsonValue->toJSONValueString();		
		res = res + str;
	}
	res = res + "\n}\n";
	return res;*/
	return "";
}

JSONValue* JSONObject::get(String key)
{
	//logger.print(tag, "\n\t JSONObject::get key=" + key);
	for (int i = 0; i < map.length(); i++) {
		JSONValue* value = (JSONValue*)map.get(i);
		//logger.print(tag, "\n\t value->key=" + value->key);
		if (value->key == key) {
			return value;
		}
		//return value;
	}
	//logger.print(tag, "\n\t JSONObject::get not found key=" + key);
	return nullptr;
}

bool JSONObject::has(String key)
{
	for (int i = 0; i < map.length(); i++) {
		JSONValue* value = (JSONValue*)map.get(i);
		if (value->key == key) {
			return true;
		}
	}
	return false;
}

String JSONObject::getString(String key)
{
	JSONValue* value = get(key);
	
	if (value->type == JSONValueType_String) {
		JSONStringValue* p = (JSONStringValue*)value;
		return p->value;
	}
	//logger.print(tag, "\n\t JSONArrayObject::getString not found =" + key);
	return "";
}

int JSONObject::getInteger(String key)
{
	JSONValue* value = get(key);

	if (value->type == JSONValueType_Integer) {
		JSONIntegerValue* p = (JSONIntegerValue*)value;
		return p->value;
	}
	//logger.print(tag, "\n\t JSONArrayObject::getInteger not found =" + key);
	return -1;
}

float JSONObject::getFloat(String key)
{
	JSONValue* value = get(key);

	if (value->type == JSONValueType_Float) {
		JSONFloatValue* p = (JSONFloatValue*)value;
		return p->value;
	}
	//logger.print(tag, "\n\t JSONArrayObject::getFloat not found =" + key);
	return -1;
}

bool JSONObject::getBool(String key)
{
	JSONValue* value = get(key);

	if (value->type == JSONValueType_Boolean) {
		JSONBoolValue* p = (JSONBoolValue*)value;
		return p->value;
	}
	//logger.print(tag, "\n\t JSONArrayObject::getBool not found =" + key);
	return false;
}

String JSONObject::getJSONArray(String key)
{
	JSONValue* value = get(key);

	if (value->type == JSONValueType_JSONArray)
	{
		JSONArrayValue* p = (JSONArrayValue*)value;
		//JSONArrayValue* p = new JSONArrayValue();
		return p->value;
	}
	//logger.print(tag, "\n\t JSONArrayObject::getArray not found =" + key);
	return "";
}


bool JSONObject::pushString(String key, String value)
{
	//logger.print(tag, String("[") + key + "]:" + value + "(String)\n");
	JSONStringValue* obj = new JSONStringValue(key,value);
	map.add(obj);
	return  true;
}

bool JSONObject::pushInteger(String key, int value)
{
	logger.print(tag, String("[") + key + String("]:") + value + "(Integer)\n");
	JSONIntegerValue* obj = new JSONIntegerValue(key, value);
	map.add(obj);
	return  true;
}

bool JSONObject::pushFloat(String key, float value)
{
	//logger.print(tag, String("[") + key + "]:" + value + "(Float)\n");
	JSONFloatValue* obj = new JSONFloatValue(key, value);
	map.add(obj);
	return  true;
}

bool JSONObject::pushBool(String key, bool value)
{
	//logger.print(tag, String("[") + key + "]:" + String(value) + "(Bool)\n");
	JSONBoolValue* obj = new JSONBoolValue(key, value);
	map.add(obj);
	return  true;
}

bool JSONObject::pushJSONArray(String key, String value)
{
	//logger.print(tag, String("[") + key + "]:" + value + "(JSONArray)\n");
	JSONArrayValue* obj = new JSONArrayValue(key, value);
	map.add(obj);
	return  true;
}

void JSONObject::parse(String json)
{
	//logger.print(tag, "\n");
	//logger.println(tag, "parse json=" + json);

	Tokener tokener(json);

	char c = tokener.nextClean();

	//logger.print(tag, "c=" + String(c) + "\n");

	if (c == '{') {


		while (c != 0) {

			
			c = tokener.nextClean();

			//logger.print(tag, "c=" + String(c) + "\n");

			if (c == 34) { // '"'

				String key = tokener.nextTo(34);

				c = tokener.nextClean();
				if (c == 58) { // ':'
					c = tokener.nextClean();

					if (c == 34) { // '"'

						char c2 = tokener.nextCleanNoCursor();
						/*if (c2 == '[') {
							String val = tokener.nextArray();
							JSONArrayObject* jarray = new JSONArrayObject(val);
							pushJSONArray(key, val);
						} else {*/

							// string
							String val = tokener.nextTo(34);	
							pushString(key, val);
						/*}*/

					}
					else if ((c >= '0' && c <= '9') //numeric 
						|| c == '-' /* '-' */
						|| c == '.' /* '.' */) { // '"'

						// number
						if (tokener.nextIsFloat()) {
							float val = tokener.nextFloat();
							pushFloat(key, val);
						}
						else {
							int val = tokener.nextInteger();
							pushInteger(key, val);
						}
						
					}
					else if (c == 't' || c == 'f') { // '"'
						// bool false
						bool val = tokener.nextBool();
						pushBool(key, val);
					}
					/*else if (c == '{') {
						throw("unexpected {");
					}*/
					else if (c == '[') { 
						String val = tokener.nextArray();
						JSONArrayObject* jarray = new JSONArrayObject(val);
						pushJSONArray(key, val);
					}
					else {
						logger.print(tag, "\n\t JSONObject::parse c = " + String(c));
						logger.print(tag, "\n\t JSONObject::parse invalid format =" + json);
						return;
					}

					c = tokener.nextClean();
					if (c == 44) { // ','
						continue;
					} 
					else if (c == '}') { // '}'
						return;
					}

				}
			}
		}
	}
}
