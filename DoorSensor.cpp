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

void DoorSensor::init()
{
	pinMode(pin, INPUT);
}

bool DoorSensor::getOpenStatus() {
	return openStatus;
}

bool DoorSensor::checkDoorStatus() {
	
	//logger.print(tag, "\n\t >>checkDoorStatus: ");

	if (digitalRead(D3) == LOW) {
		openStatus = true;
		//logger.print(tag, "\n\t >>>> DOOR OPEN");
	}
	else {
		openStatus = false;
		//logger.print(tag, "\n\t >>>> DOOR CLOSED");
	}
	return openStatus;
}

String DoorSensor::getJSONFields() {
	logger.print(tag, "\n\t>>Door::getJSONFields");
	String json = "";
	
	// specific field
	json += ",\"open\":";
	if (openStatus)
		json += "true";
	else 
		json += "false";
	//json += ",\"addr\":";
	//json += "\"" + getSensorAddress() + "\"";
	
	logger.print(tag, "\n\t<<Door::getJSONFields json=" + json);
	return json;
}


