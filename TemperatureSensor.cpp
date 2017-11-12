#include "TemperatureSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

Logger TemperatureSensor::logger;
String TemperatureSensor::tag = "TemperatureSensor";



TemperatureSensor::TemperatureSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id,pin, enabled, address, name)
{
	type = "temperaturesensor";
}

TemperatureSensor::~TemperatureSensor()
{
}

void TemperatureSensor::init()
{
	logger.print(tag, "\n\t >>init TemperatureSensor");
	//pinMode(pin, INPUT);
	logger.print(tag, "\n\t <<init TemperatureSensor");
}


bool TemperatureSensor::checkStatusChange() {
		
	return false;
}

bool TemperatureSensor::getJSON(JSONObject *jObject) {

	logger.print(tag, "\n");
	logger.println(tag, ">>getJSON");

	bool res = Sensor::getJSON(jObject);
	if (!res) return false;

	jObject->pushInteger("id", id);
	jObject->pushString("phisicaladdr", getPhisicalAddress());
	jObject->pushFloat("temperature", temperature);
	jObject->pushFloat("avtemperature", avTemperature);
	
	logger.println(tag, "<<getJSON");

	return res;
}


String TemperatureSensor::getJSONFields() {

	//logger.println(tag, ">>getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	json += String(",\"id\":");
	json += String(id) + "";
	json += ",\"phisicaladdr\":";
	json += "\"" + getPhisicalAddress() + "\"";
	json += ",\"temperature\": ";
	json += String(temperature);
	json += ",\"avtemperature\": ";
	json += String(avTemperature);

	//logger.println(tag, "<<getJSONField" + json);
	return json;
}

