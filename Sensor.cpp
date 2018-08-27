#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"
#include "MQTTMessage.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";

extern bool mqtt_publish(MQTTMessage mqttmessage);

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

Sensor::Sensor()
{
}

Sensor::~Sensor()
{
	childsensors.clear();
}

void Sensor::show() {

	/*logger.print(tag, "\n\t sensorid=");
	logger.print(tag, String(sensorid));
	logger.print(tag, "\n\t name=");
	logger.print(tag, sensorname);
	logger.print(tag, "\n\t addr= ");
	logger.print(tag, address);
	logger.print(tag, "\n\t enabled= ");
	logger.print(tag, String(enabled));
	logger.print(tag, "\n\t pin= ");
	logger.print(tag, String(pin));*/
}

String Sensor::toString()
{
	String str = "sensor: " + sensorname + ";type: " + type + ";sensorid: " + sensorid;
	if (childsensors.size() > 0)
		str += ";chidren:" + String(childsensors.size());
	return str;
}

Sensor * Sensor::getSensorFromId(int id)
{
	logger.print(tag, F("\n\t >>Sensor::getSensorFromId "));

	if (sensorid == id)
		return (Sensor*)this;
	
	if (childsensors.size() > 0) {
		for (int i = 0; i < childsensors.size(); i++) {
			//logger.print(tag, "\n\t i= " + String(i));
			Sensor* child = (Sensor*)childsensors.get(i);
			Sensor* sensor = child->getSensorFromId(id);
			if (sensor->sensorid == id)
				return (Sensor*)this;
		}
	}
	logger.println(tag, F("<<Sensor::getSensorFromId"));
	return nullptr;
}

void Sensor::getJson(JsonObject& json) {

	json["sensorid"] = sensorid;
	json["status"] = status;
	json["addr"] = address;
	logger.printJson(json);
		
	if (childsensors.size() > 0) {
		JsonArray& children = json.createNestedArray("children");
		for (int i = 0; i < childsensors.size(); i++) {
			Sensor* child = (Sensor*)childsensors.get(i);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& childjson = jsonBuffer.createObject();
			child->getJson(childjson);
			children.add(child);
		}
	}
}

void Sensor::loadChildren(JsonArray& json)
{
}

void Sensor::init()
{
}

bool Sensor::checkStatusChange()
{
	String oldStatus = status;
	if (!status.equals(oldStatus))
		return true;
	return false;
}

String Sensor::getStatusText()
{
	return status;
}

bool Sensor::receiveCommand(String command, int id, String uuid, String jsoncmd)
{
	logger.print(tag, F("\n\t >>Sensor::receiveCommand"));
	if (command.equals("requestsensorstatus")) {// richiesta stato di un singolo sensore
		logger.print(tag, F("\n\t requestsensorstatus"));
		DynamicJsonBuffer jsonBuffer;
		JsonObject& jsonresult = jsonBuffer.createObject();
		getJson(jsonresult);
		String jsonStr;
		logger.printJson(jsonresult);
		jsonresult.printTo(jsonStr);
		logger.print(tag, F("\n\t jsonstr="));
		logger.print(tag, jsonStr);
		return sendCommandResponse(uuid, jsonStr);
	}
	logger.print(tag, F("\n\t <<Sensor::receiveCommand"));
	return false;
}

bool Sensor::sendCommandResponse(String uuid, String response)
{
	logger.print(tag, "\n\t sendCommandResponse uuid=" + uuid + "response" + response);
	MQTTMessage mqttmessage;
	mqttmessage.topic = "toServer/response/" + uuid + "/success";
	mqttmessage.message = response;
	return mqtt_publish(mqttmessage);
}

