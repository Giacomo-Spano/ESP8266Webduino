#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

JSONObject DoorSensor::getJSON2()
{

	JSONObject jObject = Sensor::getJSON2();
	jObject.pushBool("open", openStatus);
	/*jObject.pushString("phisicaladdr", getPhisicalAddress());
	jObject.pushFloat("temperature", temperature);
	jObject.pushFloat("avtemperature", avTemperature);*/

	return jObject;
}


DoorSensor::DoorSensor(uint8_t pin, bool enabled, String address, String name) : Sensor(pin, enabled, address, name)
{
	type = "doorsensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;
}

DoorSensor::~DoorSensor()
{
}

void DoorSensor::init()
{
	logger.print(tag, "\n\t >>init DoorSensor");
	pinMode(pin, INPUT);
	logger.print(tag, "\n\t <<init DoorSensor");
}

bool DoorSensor::getOpenStatus() {
	return openStatus;
}

bool DoorSensor::checkStatusChange() {
	
	//logger.print(tag, "\n\t >>checkDoorStatus: ");
	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;

		bool oldStatus = openStatus;

		if (digitalRead(pin) == LOW) {
			openStatus = true;
			//logger.print(tag, "\n\t >>>> DOOR OPEN");
		}
		else {
			openStatus = false;
			//logger.print(tag, "\n\t >>>> DOOR CLOSED");
		}
		if (oldStatus != openStatus) {
			if (openStatus) 
				logger.print(tag, "\n\t >>>> DOOR OPEN");
			else
				logger.print(tag, "\n\t >>>> DOOR CLOSED");
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

/*String DoorSensor::getJSONFields(int jsontype)
{
	logger.print(tag, "\n\t>>Door::getJSONFields");
	String json = "";

	// specific field
	json += ",\"open\":";
	if (openStatus)
		json += "true";
	else
		json += "false";

	logger.print(tag, "\n\t<<Door::getJSONFields json=" );
	return json;
}*/


