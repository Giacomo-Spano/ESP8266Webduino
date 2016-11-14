#include "JSON.h"
#include "HardwareSerial.h"


JSON::JSON(String json)
{
	this->json = json;
}


JSON::~JSON()
{
}

String JSON::jsonGetString(String key) {

	/*Serial.print("json = ");
	Serial.println(json);
	Serial.print("key = ");
	Serial.println(key);*/

	key = "\"" + key + "\"";
	//Serial.print("key = ");
	//Serial.println(key);

	int index = json.indexOf(key);
	//Serial.print("index = ");
	//Serial.println(index);
	if (index < 0) return "";

	json = json.substring(index + key.length());
	//Serial.print("json = ");
	//Serial.println(json);

	index = json.indexOf("\"");
	//Serial.print("index = ");
	//Serial.println(index);
	if (index < 0) return "";

	json = json.substring(index + 1);
	//Serial.print("json = ");
	//Serial.println(json);

	int end = json.indexOf("\"", index);
	//Serial.print("end = ");
	//Serial.println(end);
	if (end < 0) return "";

	String value = json.substring(0, end);
	//Serial.print("value = ");
	//Serial.println(value);
	return value;
}

String JSON::jsonGetValue(String key) {

	/*Serial.print("json = ");
	Serial.println(json);
	Serial.print("key = ");
	Serial.println(key);*/

	key = "\"" + key + "\"";
	//Serial.print("key = ");
	//Serial.println(key);

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

	int end = json.indexOf("\"", index);
	//Serial.print("end = ");
	//Serial.println(end);
	if (end < 0)
		end = json.indexOf("}", index);
	if (end < 0)
		end = json.indexOf("[", index);

	if (end < 0) return "";

	String value = json.substring(0, end);
	value.trim();

	Serial.print("value = ");
	Serial.println(value);
	return value;
}

int JSON::jsonGetInt(String key) {
	String value = jsonGetValue(key);
	return value.toInt();
}

long JSON::jsonGetLong(String key) {
	String value = jsonGetValue(key);
	char tarray[20]; // numero di digit del long
	value.toCharArray(tarray, sizeof(tarray));
	//Serial.print("tarray = ");
	//Serial.println(tarray);
	long longval = value.toInt();
	//long longval = atol(tarray);

	//Serial.print("longval = ");
	//Serial.println(longval);
	return longval;
}

