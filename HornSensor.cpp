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

CommandResponse HornSensor::receiveCommand(String jsonStr)
{
	logger.println(tag, ">>receiveCommand=");
	CommandResponse response;

	JSON json(jsonStr);
	// actuatorId
	int actuatorId;
	if (json.has("actuatorid")) {
		actuatorId = json.jsonGetInt("actuatorid");
		logger.print(tag, "\n\t actuatorid=" + String(actuatorId));
	}

	// command
	String command = "";
	if (json.has("command")) {
		command = json.jsonGetString("command");
		logger.print(tag, "\n\t command=" + command);


		if (command.equals("teststart")) {
			logger.print(tag, "\n\t test start command");
			testMode = true;
			testOpenStatus = alarmActive;
		}
		else if (command.equals("teststop")) {
			logger.print(tag, "\n\t test stop command");
		} if (command.equals("testopen")) {
			logger.print(tag, "\n\t test open command");
			testOpenStatus = true;
		} if (command.equals("testclose")) {
			logger.print(tag, "\n\t test close command");
			testMode = false;
		}

		if (json.has("uuid")) {
			response.uuid = json.jsonGetString("uuid");
		}
		response.result = "success";// response_success;
	}

	logger.println(tag, "<<receiveCommand res="/* + String(res)*/);
	return response;
}


