#include "JSON.h"
#include "HardwareSerial.h"

extern Logger logger;

JSON::JSON(String json)
{
	this->jsonString = json;

	String tag = "getNtpTime";
}


JSON::~JSON()
{
}

String JSON::jsonGetString(String key) {

	//logger.print(tag, "\n\t>>jsonGetString : " + key);

	key = "\"" + key + "\"";
	String json = jsonString;

	int index = json.indexOf(key);
	if (index < 0) return "";

	json = json.substring(index + key.length());
	//logger.print(tag, "\n\tjson=" + key);

	index = json.indexOf("\"");
	//logger.print(tag, "\n\tindex=" + String(index));
	if (index < 0) return "";

	json = json.substring(index + 1);
	//logger.print(tag, "\n\tjson=" + json);

	int end = json.indexOf("\"", index);
	//logger.print(tag, "\n\tend=" + String(end));
	if (end < 0) return "";

	String value = json.substring(0, end);
	//logger.print(tag, "\n\tvalue=" + value);
	return value;
}

bool JSON::has(String key) {

	String json = jsonString;

	key = "\"" + key + "\"";

	int index = json.indexOf(key);
	if (index < 0) {
		//logger.print(tag, "\n\tjsonString " + jsonString);
		logger.print(tag, "\n\tkey " + key + "not found");
		return false;
	}

	json = json.substring(index + key.length());
	index = json.indexOf(":");
	if (index < 0) {
		logger.print(tag, "\n\t: char not found");
		return false;
	}

	json = json.substring(index + 1);


	/*int end = json.indexOf("\"", index);
	if (end < 0)
		end = json.indexOf("}", index);
	if (end < 0)
		end = json.indexOf("[", index);

	if (end < 0) {
		return false;
	}*/

	logger.print(tag, "\n\tkey " + key + "found");
	return true;
	//String value = json.substring(0, end);
	//value.trim();

	//return value;
}



String JSON::getRightOfKey(String key) {
	// ritorna la stringa a destra di 'key' del json

	String json = jsonString;

	/*Serial.print("\n\t getRightOfKey json = ");
	Serial.println(json);
	Serial.print("key = ");
	Serial.println(key);*/

	key = "\"" + key + "\"";
	/*Serial.print("key = ");
	Serial.println(key);*/

	int index = json.indexOf(key);
	//Serial.print("index = ");
	//Serial.println(index);
	if (index < 0) return "";

	json = json.substring(index + key.length());
	//Serial.print("json = ");
	//Serial.println(json);


	index = json.indexOf(":");
	//Serial.print("index = ");
	//Serial.println(index);
	if (index < 0) return "";

	json = json.substring(index + 1);
	//Serial.print("json = ");
	//Serial.println(json);

	return json;
}

String JSON::getNum(String key) {
	// ritorna il valore di una key senza apici

	logger.print(tag, "\n\tgetNum key = ");
	logger.print(tag, key);

	String json = getRightOfKey(key);
	logger.print(tag, "\n\tjson= ");
	logger.print(tag, json);

	int end = json.indexOf(",");
	if (end < 0) {
		end = json.indexOf("}");
		if (end < 0) {
			return "";
		}
	}
	
	String value = json.substring(0, end);
	logger.print(tag, "\n\tvalue= "+ value);
	value.trim();
	logger.print(tag, "\n\tvalue= " + value);
	return value;
}

bool JSON::getBool(String key) {
	// ritorna il valore di una key senza apici

	//logger.print(tag, "\n\tkey = ");
	//logger.print(tag, key);

	String json = getRightOfKey(key);
	//logger.print(tag, "\n\tjson= ");
	//logger.print(tag, json);

	int end = json.indexOf(",");
	if (end < 0) {
		end = json.indexOf("}");
		if (end < 0) {
			//logger.print(tag, "\n\treturn false");
			return false;
		}
	}
	
	String value = json.substring(0, end);
	value.trim();

	//logger.print(tag, "\n\tvalue="+value);
	if (value.equals("true"))
		return true;
	else
		return false;
}

String JSON::getString(String key) {
	// ritorna il valore di una key con apici

	//logger.print(tag, "\n\tgetString key = ");
	//logger.print(tag, key);

	String json = getRightOfKey(key);
	if (json.equals(""))
		return "";

	int end = json.indexOf(",");
	if (end < 0) {
		end = json.indexOf("}");
		if (end < 0)
			return "";
	}

	if (end < 0) return "";

	String value = json.substring(0, end);
	value.trim();
	return value;
}

float JSON::jsonGetFloat(String key) {
	//logger.print(tag, "\n\tjsonGetFloat = ");
	//logger.print(tag, key);

	String value = getNum(key);
	if (value.equals(""))
		return 0;
	//logger.print(tag, "\n\tvalue = ");
	//logger.print(tag, value);
	return value.toFloat();
}

int JSON::jsonGetInt(String key) {
	logger.print(tag, "\n\tjsonGetLong = ");
	logger.print(tag, key);

	String value = getNum(key);
	if (value.equals(""))
		return 0;
	return value.toInt();
}

bool JSON::jsonGetBool(String key) {
	//logger.print(tag, "\n\tjsonGetBool: ");
	//logger.print(tag, key);

	bool value = getBool(key);
	return value;
}


long JSON::jsonGetLong(String key) {
	//logger.print(tag, "\n\tjsonGetLong: ");
	//logger.print(tag, key);

	String value = getNum(key);
	//logger.print(tag, "\n\tvalue = ");
	//logger.print(tag, value);
	if (value.equals(""))
		return 0;

	char tarray[20]; // numero di digit del long
	value.toCharArray(tarray, sizeof(tarray));
	//logger.print(tag, "\n\ttarray = ");
	//logger.print(tag, tarray);
	long longval = value.toInt();
	//logger.print(tag, "\n\tlongval = ");
	//logger.print(tag, longval);
	return longval;
}

