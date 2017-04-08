// 
// 
// 

#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

DoorSensor::DoorSensor()
{
	type = "doorsensor";
}

DoorSensor::~DoorSensor()
{
}

bool DoorSensor::getOpenStatus() {
	return openStatus;
}

void DoorSensor::readStatus() {

	logger.print(tag, "\n\t>>readStatus: ");
	
	
	logger.print(tag, "\n\tstatus  is: ");

	logger.print(tag, "\n\t<<readStatus: ");
}

String DoorSensor::getJSONFields() {
	logger.print(tag, "\n\t>>Door::getJSONFields");
	String json = "";
	// specific field
	json += ",\"open\":";
	json += String(getOpenStatus());
	logger.print(tag, "\n\t<<Door::getJSONFields json=" + json);
	return json;
}


