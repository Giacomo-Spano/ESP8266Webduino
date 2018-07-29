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
	logger.print(tag, "\n\t >>SensorFactory::createSensor type=" + type);

	Sensor* sensor = nullptr;
	if (type.equals("temperaturesensor")) {
		logger.print(tag, "\n\t creating temperature sensor");
		sensor = new TemperatureSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("heatersensor")) {
		logger.print(tag, "\n\t creating heatersensor sensor");
		sensor = new HeaterSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("doorsensor")) {
		logger.print(tag, "\n\t creating doorsensor sensor");
		sensor = new DoorSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("onewiresensor")) {
		logger.print(tag, "\n\t creating onewiresensor sensor");
		sensor = new OnewireSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("hornsensor")) {
		logger.print(tag, "\n\t creating hornsensor sensor");
		sensor = new HornSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("rfidsensor")) {
		logger.print(tag, "\n\t creating rfidsensor sensor");
		sensor = new RFIDSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("irsensor")) {
		logger.print(tag, "\n\t creating irsensor sensor");
		sensor = new IRSensor(id, pin, enabled, address, name);
	}
	else if (type.equals("irreceivesensor")) {
		logger.print(tag, "\n\t creating irreceivesensor sensor");
		sensor = new IRReceiveSensor(id, pin, enabled, address, name);
	}

	logger.println(tag, "createSensor type=" + type);
	return sensor;
}

Sensor * SensorFactory::createSensor(JSONObject* json)
{
	logger.print(tag, "\n");
	logger.println(tag, "\n\t >>createSensor  json = " + logger.formattedJson(json->toString()));
	logger.printFreeMem(tag, "start create sensor");

	int sensorid;
	String type;
	String address;
	uint8_t pin = 0;
	bool enabled = true;
	String name = "";
		
	
	if (!json->has("type") || !json->has("subaddress") || !json->has("id")) {
		logger.print(tag, "\n\t invalid address and typ=");
		return nullptr;
	}
	type = json->getString("type");
	address = json->getString("subaddress");
	sensorid = json->getInteger("id");
	
	if (json->has("pin")) {
		String strPin = json->getString("pin");
		pin = Shield::pinFromStr(strPin);
	}

	if (json->has("pin")) {
		String strPin = json->getString("pin");
		pin = Shield::pinFromStr(strPin);
	}
	if (json->has("enabled")) {
		enabled = json->getBool("enabled");
	}
	if (json->has("name")) {
		name = json->getString("name");
	}

	logger.print(tag, "\n\t type=" + type);
	logger.print(tag, "\n\t addr=" + address);
	logger.print(tag, "\n\t sensorid=" + String(sensorid));
	logger.print(tag, "\n\t pin=" + String(pin));
	logger.print(tag, "\n\t enabled=" + String(enabled));
	logger.print(tag, "\n\t name=" + name);


	Sensor* sensor = nullptr;

	if (type.equals("temperaturesensor")) {
		logger.print(tag, "\n\t creating temperature sensor");
		sensor = new TemperatureSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals("heatersensor")) {
		logger.print(tag, "\n\t creating heatersensor sensor");
		sensor = new HeaterSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals("doorsensor")) {
		logger.print(tag, "\n\t creating doorsensor sensor");
		sensor = new DoorSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals("onewiresensor")) {
		logger.print(tag, "\n\t creating onewiresensor sensor");
		sensor = new OnewireSensor(sensorid,pin, enabled, address, name);
	}
	else if (type.equals("hornsensor")) {
		logger.print(tag, "\n\t creating doorsensor sensor");
		sensor = new HornSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals("rfidsensor")) {
		logger.print(tag, "\n\t creating rfidsensor sensor");
		sensor = new RFIDSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals("irsensor")) {
		logger.print(tag, "\n\t creating irsensor sensor");
		sensor = new IRSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals("irreceivesensor")) {
		logger.print(tag, "\n\t creating irreceivesensor sensor");
		sensor = new IRReceiveSensor(sensorid, pin, enabled, address, name);
	}
	else {
		return nullptr;
	}

	sensor->init();
	if (json->has("childsensors")) {
		String children = json->getJSONArray("childsensors");
		logger.print(tag, "\n\t children=" + children);
		JSONArray jarray(children);
		sensor->loadChildren(jarray);
	}
	
	logger.printFreeMem(tag, "end create sensor");
	logger.println(tag, "<<SensorFactory::createSensor");
	return sensor;
}


