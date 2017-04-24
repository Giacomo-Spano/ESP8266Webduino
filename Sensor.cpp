#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";

Sensor::Sensor(uint8_t pin, bool enabled, String address, String name)
{
	this->pin = pin;
	this->enabled = enabled;
	this->address = address;
	this->sensorname = name;

	checkStatus_interval = 10000;//60000; // 60 seconds
	lastCheckStatus = 0;// = 0;//-flash_interval;
}

/*Sensor::Sensor()
{
}*/

Sensor::~Sensor()
{
}

void Sensor::show() {

		logger.print(tag, "\n\t name=");
		logger.print(tag,sensorname);
		logger.print(tag, "\n\t addr= ");
		logger.print(tag, address);
		logger.print(tag, "\n\t enabled= ");
		logger.print(tag, String(enabled));
		logger.print(tag, "\n\t pin= ");
		logger.print(tag, String(pin));
}

String Sensor::getJSON() {

	logger.print(tag, "\n\t>>getJSON");

	String json = "";
	json += "{";

	// common field
	json += "\"type\":\""+ type + "\"";
	json += ",\"name\":\"";
	json += String(sensorname) + "\"";
	json += ",\"enabled\":";
	if (enabled == true)
		json += "true";
	else
		json += "false";
	json += ",\"pin\":\"" + Shield::getStrPin(pin) + "\"";
	json += ",\"addr\":\"";
	json += address + "\"";
	
	// get custom json field
	json += getJSONFields();
	
	json += "}";

	logger.print(tag, "\n\t<<getJSON json=" + json);
	return json;
}

String Sensor::getJSONFields()
{
	return String();
}

void Sensor::init()
{
}

bool Sensor::checkStatusChange()
{
}

String Sensor::sendCommand(String json)
{
	return String();
}

/*String Sensor::getSensorAddress() {

	return address;
}*/


