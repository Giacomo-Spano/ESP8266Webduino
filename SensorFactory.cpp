#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "HeaterSensor.h"
#include "DoorSensor.h"
#include "OnewireSensor.h"
#include "HornSensor.h"
#include "RFIDSensor.h"
#include "IRSensor.h"
#include "IRReceiveSensor.h"
#include "JSONObject.h"
#include "Shield.h"
//#include "Util.h"
//#include "ESP8266Webduino.h"

Logger SensorFactory::logger;
String SensorFactory::tag = "SensorFactory";

SensorFactory::SensorFactory()
{
}

SensorFactory::~SensorFactory()
{
}

Sensor * SensorFactory::createSensor(int id, String type, uint8_t pin, bool enabled, String address, String name)
{
	logger.print(tag, F("\n\t >>SensorFactory::createSensor type="));
	logger.print(tag, type);

	Sensor* sensor = nullptr;
	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("F(heatersensor")) {
		logger.print(tag, "\n\t creating heatersensor sensor");
		sensor = new HeaterSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("doorsensor"))) {
		logger.print(tag, "\n\t creating doorsensor sensor");
		sensor = new DoorSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("onewiresensor"))) {
		logger.print(tag, F("\n\t creating onewiresensor sensor"));
		sensor = new OnewireSensor(id, pin, enabled, address, name);
	}
	else if (type.equals((F("hornsensor")))) {
		logger.print(tag, F("\n\t creating hornsensor sensor"));
		sensor = new HornSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating rfidsensor sensor"));
		sensor = new RFIDSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("irsensor"))) {
		logger.print(tag, "\n\t creating irsensor sensor");
		sensor = new IRSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("irreceivesensor"))) {
		logger.print(tag, F("\n\t creating irreceivesensor sensor"));
		sensor = new IRReceiveSensor(id, pin, enabled, address, name);
	}

	logger.println(tag, F("createSensor type="));
	logger.println(tag, type);
	return sensor;
}

#ifdef dopo
Sensor * SensorFactory::createSensor(JSONObject* json)
{
	logger.print(tag, F("\n"));
	logger.println(tag, F("\n\t >>createSensor  json = "));
	logger.println(tag, logger.formattedJson(json->toString()));
	logger.printFreeMem(tag, F("start create sensor"));

	int sensorid;
	String type;
	String address;
	uint8_t pin = 0;
	bool enabled = true;
	String name = "";
		
	
	if (!json->has(F("type")) || !json->has(F("subaddress")) || !json->has(F("id"))) {
		logger.print(tag, F("\n\t invalid address and typ="));
		return nullptr;
	}
	type = json->getString(F("type"));
	address = json->getString(F("subaddress"));
	sensorid = json->getInteger(F("id"));
	
	if (json->has(F("pin"))) {
		String strPin = json->getString(F("pin"));
		pin = Shield::pinFromStr(strPin);
	}

	if (json->has(F("pin"))) {
		String strPin = json->getString(F("pin"));
		pin = Shield::pinFromStr(strPin);
	}
	if (json->has(F("enabled"))) {
		enabled = json->getBool(F("enabled"));
	}
	if (json->has(F("name"))) {
		name = json->getString(F("name"));
	}

	logger.print(tag, "\n\t type=");
	logger.print(tag, type);
	logger.print(tag, F("\n\t addr=" ));
	logger.print(tag, address);
	logger.print(tag, F("\n\t sensorid=" ));
	logger.print(tag, sensorid);
	logger.print(tag, F("\n\t pin="));
	logger.print(tag, String(pin));
	logger.print(tag, F("\n\t enabled="));
	logger.print(tag, String(enabled));
	logger.print(tag, F("\n\t name="));
	logger.print(tag, name);


	Sensor* sensor = nullptr;

	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals("heatersensor")) {
		logger.print(tag, F("\n\t creating heatersensor sensor"));
		sensor = new HeaterSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals(F("doorsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new DoorSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals(F("onewiresensor"))) {
		logger.print(tag, F("\n\t creating onewiresensor sensor"));
		sensor = new OnewireSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals(F("hornsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new HornSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating rfidsensor sensor"));
		sensor = new RFIDSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irsensor"))) {
		logger.print(tag, F("\n\t creating irsensor sensor"));
		sensor = new IRSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irreceivesensor"))) {
		logger.print(tag, F("\n\t creating irreceivesensor sensor"));
		sensor = new IRReceiveSensor(sensorid, pin, enabled, address, name);
	}
	else {
		return nullptr;
	}

	sensor->init();
	if (json->has(F("childsensors"))) {
		String children = json->getJSONArray(F("childsensors"));
		logger.print(tag, F("\n\t children="));
		logger.print(tag, children);
		JSONArray jarray(children);
		sensor->loadChildren(jarray);
	}
	
	logger.printFreeMem(tag, F("end create sensor"));
	logger.println(tag, F("<<SensorFactory::createSensor"));
	return sensor;
}
#endif

Sensor * SensorFactory::createSensor(JsonObject& json)
{
	logger.print(tag, F("\n"));
	logger.println(tag, F("\n\t >>createSensor2  json = "));
	//logger.println(tag, logger.formattedJson(json->toString()));
	json.printTo(Serial);
	logger.printFreeMem(tag, F("start create sensor"));

	//int sensorid;
	//String type;
	//String address;
	uint8_t pin = 0;
	bool enabled = true;
	String name = "";


	if (!json.containsKey("type") || !json.containsKey("subaddress") || !json.containsKey("id")) {
		logger.print(tag, F("\n\t invalid address and typ="));
		return nullptr;
	}
	String type = json["type"];
	String address = json[F("subaddress")];
	int sensorid = json["id"];	
	if (json.containsKey(F("pin"))) {
		String strPin = json["pin"];
		pin = Shield::pinFromStr(strPin);
	}
	if (json.containsKey(F("enabled"))) {
		enabled = json["enabled"];
	}
	if (json.containsKey(F("name"))) {
		String str = json["name"];
		name = str;
	}
	logger.print(tag, "\n\t type=");
	logger.print(tag, type);
	logger.print(tag, F("\n\t addr="));
	logger.print(tag, address);
	logger.print(tag, F("\n\t sensorid="));
	logger.print(tag, sensorid);
	logger.print(tag, F("\n\t pin="));
	logger.print(tag, String(pin));
	logger.print(tag, F("\n\t enabled="));
	logger.print(tag, String(enabled));
	logger.print(tag, F("\n\t name="));
	logger.print(tag, name);
	
	Sensor* sensor = nullptr;
	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals("heatersensor")) {
		logger.print(tag, F("\n\t creating heatersensor sensor"));
		sensor = new HeaterSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("doorsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new DoorSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("onewiresensor"))) {
		logger.print(tag, F("\n\t creating onewiresensor sensor"));
		sensor = new OnewireSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("hornsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new HornSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating rfidsensor sensor"));
		sensor = new RFIDSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irsensor"))) {
		logger.print(tag, F("\n\t creating irsensor sensor"));
		sensor = new IRSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irreceivesensor"))) {
		logger.print(tag, F("\n\t creating irreceivesensor sensor"));
		sensor = new IRReceiveSensor(sensorid, pin, enabled, address, name);
	}
	else {
		return nullptr;
	}

	sensor->init();
	if (json.containsKey("childsensors")) {
		
		JsonArray& children = json["childsensors"];
		logger.print(tag, F("\n\t children="));
		//logger.print(tag, children);
		children.printTo(Serial);
		//JSONArray jarray(children);
		sensor->loadChildren(children);
	}

	logger.printFreeMem(tag, F("end create sensor"));
	logger.println(tag, F("<<SensorFactory::createSensor"));
	return sensor;
}

