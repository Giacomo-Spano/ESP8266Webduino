#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

extern bool mqtt_publish(String topic, String message);


Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

bool DoorSensor::getJSON(JSONObject *jObject)
{
	//logger.print(tag, "\n");
	//logger.println(tag, ">>getJSON");

	bool res = Sensor::getJSON(jObject);
	if (!res) return false;

	res = jObject->pushBool("open", openStatus);

	//logger.println(tag, "<<getJSON");
	return true;
}

String DoorSensor::getJSONFields() {

	logger.println(tag, ">>getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	if (openStatus)
		json += String(",\"open\":true");
	else
		json += String(",\"open\":false");

	logger.println(tag, "<<getJSONFields");
	return json;
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
		//logger.print(tag, "\n\t >>checkDoorStatus: ");
		lastCheckStatus = currMillis;

		bool oldStatus = openStatus;

		if (testMode) {
			openStatus = testOpenStatus;
		}
		else {

			if (digitalRead(pin) == LOW) {
				openStatus = true;
			}
			else {
				openStatus = false;
			}
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

CommandResponse DoorSensor::receiveCommand(String jsonStr)
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
			testOpenStatus = openStatus;
		} else if (command.equals("teststop")) {
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


