#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";

Sensor::Sensor(int id, uint8_t pin, bool enabled, String address, String name)
{
	this->sensorid = id;
	this->pin = pin;
	this->enabled = enabled;
	this->address = address;
	this->sensorname = name;

	checkStatus_interval = 10000;//60000; // 60 seconds
	lastCheckStatus = 0;// = 0;//-flash_interval;
}

Sensor::~Sensor()
{
	childsensors.clearAll();
}

void Sensor::show() {

	logger.print(tag, "\n\t sensorid=");
	logger.print(tag, String(sensorid));
	logger.print(tag, "\n\t name=");
	logger.print(tag, sensorname);
	logger.print(tag, "\n\t addr= ");
	logger.print(tag, address);
	logger.print(tag, "\n\t enabled= ");
	logger.print(tag, String(enabled));
	logger.print(tag, "\n\t pin= ");
	logger.print(tag, String(pin));
}

String Sensor::toString()
{
	String str = "sensor: " + sensorname + ";type: " + type + ";sensorid: " + sensorid;
	if (childsensors.count > 0)
		str += ";chidren:" + String(childsensors.count);
	return str;
}

bool Sensor::getJSON(JSONObject *jObject) {
	logger.print(tag, "\n");
	logger.println(tag, ">>getJSON");
	
	jObject->pushString("type", type);
	jObject->pushString("name", sensorname);
	jObject->pushBool("enabled", enabled);
	jObject->pushString("pin", Shield::getStrPin(pin));
	jObject->pushString("addr", address);

	// child sensors
	if (childsensors.length() > 0) {
		logger.println(tag, "\n\t add child sensors JSON");
		String childrenJsonArray = "[";
		JSONObject childJson;
		for (int i = 0; i < childsensors.length(); i++) {
			Sensor* child = (Sensor*)childsensors.get(i);
			//JSONObject childJson;
			childJson.map.clearAll();
			child->getJSON(&childJson);
			if (i > 0) 
				childrenJsonArray += ",";
			childrenJsonArray += childJson.toString();
		}
		childrenJsonArray += "]";
		JSONArray jarray(childrenJsonArray);
		jObject->pushJSONArray("childsensors", childrenJsonArray);
		logger.println(tag, "\n\t child sensors JSON added\n");
	}

	logger.println(tag, "<<getJSON");
	return true;
}

String Sensor::getJSONFields() {

	logger.println(tag, ">>getJSONFields");

	String json = "";
	json += "\"type\":\"" + type + "\",";
	json += "\"name\":\"" + sensorname + "\",";
	json += "\"sensorid\":" + String(sensorid) + ",";
	json += "\"enabled\":";
	if (enabled)
		json += "true,";
	else
		json += "false,";
	json += "\"pin\":\"" + Shield::getStrPin(pin) + "\",";
	if (testMode) {
		json += "\"testmode\":true,";
	}
	json += "\"addr\":\"" + address + "\"";
	

	logger.println(tag, ">>getJSONFields" + json);
	return json;
}

String Sensor::getChildren() {

	logger.print(tag, "\n");
	logger.println(tag, ">>getChildren child num=" + String(childsensors.length()));

	String json = "";
	if (childsensors.length() > 0) {

		json += ",\"childsensors\":";
		json += "[";
		for (int i = 0; i < childsensors.length(); i++) {
			if (i > 0)
				json += ",";
			Sensor* child = (Sensor*)childsensors.get(i);
			json += child->getJSON();			
		}
		json += "]";
	}
	logger.println(tag, "<<getChildren" + json);
	return json;
}

String Sensor::getJSON() {

	logger.print(tag, "\n");
	logger.println(tag, ">>getJSON");

	String json = "{";	
	json += getJSONFields();
	// child sensors
	json += getChildren();	
	json += "}";

	logger.println(tag, "<<getJSON " + json);
	return json;
}

void Sensor::loadChildren(JSONArray& json)
{
}

void Sensor::init()
{
}

bool Sensor::checkStatusChange()
{
}

CommandResponse Sensor::receiveCommand(String json)
{
	CommandResponse response;
	return response;
}
