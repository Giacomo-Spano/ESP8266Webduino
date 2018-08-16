#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

extern bool mqtt_publish(String topic, String message);


Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

void DoorSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	//json["card"] = lastcard;
}

DoorSensor::DoorSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
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
	logger.print(tag, F("\n\t >>init DoorSensor"));
	pinMode(pin, INPUT);
	logger.print(tag, F("\n\t <<init DoorSensor"));
}

bool DoorSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	if (timeDiff > checkStatus_interval) {
		//logger.print(tag, "\n\t >>>> checkStatusChange - timeDiff > checkStatus_interval");
		lastCheckStatus = currMillis;
		String oldStatus = status;

		if (digitalRead(pin) == LOW) {
			status = STATUS_DOOROPEN;
		}
		else {
			status = STATUS_DOORCLOSED;
		}

		if (!status.equals(oldStatus)) {
			if (status.equals(STATUS_DOOROPEN))
				logger.print(tag, "\n\t >>>> DOOR OPEN");
			else if (status.equals(STATUS_DOORCLOSED))
				logger.print(tag, "\n\t >>>> DOOR CLOSED");
			return true;
		}
	}
	return false;
}

bool DoorSensor::receiveCommand(String command, int id, String uuid, String json)
{
	logger.print(tag, F("\n\t >>DoorSensor::receiveCommand="));
	bool res = Sensor::receiveCommand(command, id, uuid, json);
	//logger.println(tag, ">>receiveCommand=");
	//logger.print(tag, "\n\t command=" + command);

	/*if (command.equals("teststart")) {
		logger.print(tag, "\n\t test start command");
		testMode = true;
		testOpenStatus = openStatus;
	}
	else if (command.equals("teststop")) {
		logger.print(tag, "\n\t test stop command");
	}
	else if (command.equals("testopen")) {
		logger.print(tag, "\n\t test open command");
		testOpenStatus = true;
	}
	else if (command.equals("testclose")) {
		logger.print(tag, "\n\t test close command");
		testMode = false;
	}*/

	logger.print(tag, F("\n\t <<DoorSensor::receiveCommand="));
	return res;
}
