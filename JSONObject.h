#pragma once

#ifdef ESP_DEBUG
#include "Emulator\ESPEmulator.h"
#endif

#include "List.h"
#include "JSONValue.h"
#include "JSONArrayValue.h"
#include "Logger.h"

class JSONObject : public ObjectClass
{
private:
	static String tag;
	static Logger logger;

public:

	JSONObject();
	JSONObject(String json);
	~JSONObject();

	List map;

	void parse(String json);
	bool pushString(String key, String value);
	bool pushInteger(String key, int value);
	bool pushFloat(String key, float value);
	bool pushBool(String key, bool value);
	bool pushJSONArray(String key, String value);

	bool has(String key);
	JSONValue* get(String key);
	String getString(String key);
	int getInteger(String key);
	float getFloat(String key);
	bool getBool(String key);
	String getJSONArray(String key);

	String toString();
	String toFormattedString();

};

