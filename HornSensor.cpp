#include "HornSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

//extern bool mqtt_publish(String topic, String message);


Logger HornSensor::logger;
String HornSensor::tag = "HornSensor";

void HornSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["alarmactive"] = alarmActive;
}

HornSensor::HornSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	type = "hornsensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;
}

HornSensor::~HornSensor()
{
}

void HornSensor::init()
{
	logger.print(tag, F("\n\t >>init HornSensor"));
	//pinMode(pin, INPUT);
	logger.print(tag, F("\n\t <<init HornSensor"));
}

bool HornSensor::getAlarmActiveStatus() {
	return alarmActive;
}

bool HornSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;

		bool oldalarmActive = alarmActive;

		
	}
	return false;
}

bool HornSensor::receiveCommand(String command, int id, String uuid, String json)
{
	logger.print(tag, F("\n\t >>HornSensor::receiveCommand="));
	bool res = Sensor::receiveCommand(command, id, uuid, json);
	//logger.println(tag, ">>receiveCommand=");
	//logger.print(tag, "\n\t command=" + command);

	if (command.equals("teststart")) {
		//logger.print(tag, "\n\t test start command");
	}
	else if (command.equals("teststop")) {
		//logger.print(tag, "\n\t test stop command");
	}
	else if (command.equals("testopen")) {
		//logger.print(tag, "\n\t test open command");
	}
	else if (command.equals("testclose")) {
		//logger.print(tag, "\n\t test close command");
	}
	logger.print(tag, F("\n\t >>HornSensor::receiveCommand="));
	return res;
}

