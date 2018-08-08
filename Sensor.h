#pragma once
#include "Arduino.h"
#include "Logger.h"
#include "ObjectClass.h"
#include "List.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "CommandResponse.h"

class Sensor : public ObjectClass
{
private:
	static String tag;
	static Logger logger;

	/*static const int sensorNameLen = 20;
	static const int sensorAddressLen = 20;
	static const int sensorTypeLen = 20;
	*/	

	const int json_full = 1;
	const int json_settings = 2;

public:
	Sensor(int id, uint8_t pin, bool enabled, String address, String name);
	~Sensor();

	const String STATUS_IDLE = "idle";

	List childsensors;
	int sensorid;
	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	String address;
	String status = STATUS_IDLE;

	bool testMode = false;
	bool lastUpdateStatusFailed = false;
	unsigned lastUpdateStatus = 0;
	

	int checkStatus_interval = 1000;	// il valore corretto per ogni tipo di sensore
									// è impostato nel costruttore
									// default 1 secondo
	int updateStatus_interval = 60000; // intervallo minimo di aggiornamentto 
									// default 1 minuto
	unsigned long lastCheckStatus;// = 0;//-flash_interval;
	
	String getChildren();
	Sensor* getSensorFromId(int id);

	virtual bool getJSON(JSONObject *jObject);
	virtual String getJSONFields();	
	virtual String getJSON();
	virtual void loadChildren(JSONArray& json);
	virtual void show();
	virtual String toString();	
	virtual void init();
	virtual bool checkStatusChange();
	virtual String getStatusText();
	virtual bool receiveCommand(String command, int actuatorId, String uuid, String json);
	bool sendCommandResponse(String uuid, String response);
};

