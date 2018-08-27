#pragma once
#include "Arduino.h"
#include "Logger.h"
#include "CommandResponse.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <LinkedList.h>

class Sensor/* : public ObjectClass*/
{
private:
	static String tag;
	static Logger logger;

public:
	Sensor();
	Sensor(int id, uint8_t pin, bool enabled, String address, String name);
	~Sensor();

	const String STATUS_IDLE = "idle";
	//const String STATUS_OFFLINE = "offline";

	//List childsensors;
	LinkedList<Sensor*> childsensors = LinkedList<Sensor*>();
	int sensorid;
	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	String address;
	String status = STATUS_IDLE;

	//bool testMode = false;
	//bool lastUpdateStatusFailed = false;
	unsigned lastUpdateStatus = 0;
	
	int checkStatus_interval = 1000;	// il valore corretto per ogni tipo di sensore
									// è impostato nel costruttore
									// default 1 secondo
	int updateStatus_interval = 60000; // intervallo minimo di aggiornamentto 
									// default 1 minuto
	unsigned long lastCheckStatus;// = 0;//-flash_interval;
	Sensor* getSensorFromId(int id);
	virtual void getJson(JsonObject& json);
	virtual void loadChildren(JsonArray& jsonarray);
	virtual void show();
	virtual String toString();	
	virtual void init();
	virtual bool checkStatusChange();
	virtual String getStatusText();
	virtual bool receiveCommand(String command, int actuatorId, String uuid, String json);
	bool sendCommandResponse(String uuid, String response);
};

