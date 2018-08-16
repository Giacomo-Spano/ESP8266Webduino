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
	//logger.print(tag, F("\n\t >>init TemperatureSensor");
	//pinMode(pin, INPUT);
	//logger.print(tag, "\n\t <<init TemperatureSensor");
}

bool TemperatureSensor::checkStatusChange() {
		
	return false;
}

String TemperatureSensor::getStatusText()
{
	String text = "" + String(temperature) + "°C";
	return text;
}

void TemperatureSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["temp"] = temperature;
}
