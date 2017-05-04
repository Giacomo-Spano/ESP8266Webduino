#include "TemperatureSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

Logger TemperatureSensor::logger;
String TemperatureSensor::tag = "TemperatureSensor";



TemperatureSensor::TemperatureSensor(uint8_t pin, bool enabled, String address, String name) : Sensor(pin, enabled, address, name)
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

JSONObject TemperatureSensor::getJSON2() {

	logger.print(tag, "\n\t>>TemperatureSensorgetJSON2");

	JSONObject jObject = Sensor::getJSON2();
	jObject.pushInteger("id", id);
	jObject.pushString("phisicaladdr", getPhisicalAddress());
	jObject.pushFloat("temperature", temperature);
	jObject.pushFloat("avtemperature", avTemperature);
	
	logger.print(tag, "\n\t<<TemperatureSensor::getJSON2");

	return jObject;
}

String TemperatureSensor::getJSONFields(int jsontype) {

	logger.print(tag, "\n\TemperatureSensor::>>TemperatureSensor::getJSONFields");
	String json = "";

	// specific field
	json += ",\"id\":";
	json += String(id) + "";
	json += ",\"phisicaladdr\":";
	json += "\"" + getPhisicalAddress() + "\"";
	json += ",\"temperature\":";
	json += String(temperature);
	json += ",\"avtemperature\":";
	json += String(avTemperature);

	logger.print(tag, "\n\TemperatureSensor::<<TemperatureSensor::getJSONFields json=" + json);
	return json;
}
