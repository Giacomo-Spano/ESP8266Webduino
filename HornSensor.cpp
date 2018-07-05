#include "HornSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

extern bool mqtt_publish(String topic, String message);


Logger HornSensor::logger;
String HornSensor::tag = "HornSensor";

bool HornSensor::getJSON(JSONObject *jObject)
{
	//logger.print(tag, "\n");
	//logger.println(tag, ">>HornSensor::getJSON");

	bool res = Sensor::getJSON(jObject);
	if (!res) return false;

	//res = jObject->pushBool("alarmactive", alarmActive);

	//logger.println(tag, "<<HornSensor::getJSON");
	return true;
}

String HornSensor::getJSONFields() {

	//logger.println(tag, ">>HornSensor::getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	if (alarmActive)
		json += String(",\"alarmactive\":true");
	else
		json += String(",\"alarmactive\":false");

	//logger.println(tag, "<<HornSensor::getJSONFields");
	return json;
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
	logger.print(tag, "\n\t >>init HornSensor");
	//pinMode(pin, INPUT);
	logger.print(tag, "\n\t <<init HornSensor");
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
	bool res = Sensor::receiveCommand(command, id, uuid, json);
	logger.println(tag, ">>receiveCommand=");
	logger.print(tag, "\n\t command=" + command);

	if (command.equals("teststart")) {
		logger.print(tag, "\n\t test start command");
	}
	else if (command.equals("teststop")) {
		logger.print(tag, "\n\t test stop command");
	}
	else if (command.equals("testopen")) {
		logger.print(tag, "\n\t test open command");
	}
	else if (command.equals("testclose")) {
		logger.print(tag, "\n\t test close command");
	}
	logger.println(tag, "<<receiveCommand res="/* + String(res)*/);
	return res;
}

