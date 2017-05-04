#pragma once
#include "Arduino.h"
#include "Logger.h"
#include "ObjectClass.h"
#include "List.h"
#include "JSONObject.h"
#include "JSONArray.h"

class Sensor : public ObjectClass
{
private:
	static String tag;
	static Logger logger;


public:
	Sensor(uint8_t pin, bool enabled, String address, String name);
	~Sensor();
	
	const int json_full = 1;
	const int json_settings = 2;

	List childsensors;

	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	String address;

	int checkStatus_interval = 20000;//60000; // 60 seconds
	unsigned long lastCheckStatus;// = 0;//-flash_interval;

	//virtual String getJSON();
	virtual JSONObject getJSON2();
	virtual void loadChildren(JSONArray json);
	//virtual String getJSON(int type);
	//virtual String getJSONFields(int jsontype);
	
	static const int sensorNameLen = 20;
	static const int sensorAddressLen = 20;
	static const int sensorTypeLen = 20;

	virtual void show();
	
	virtual void init();
	virtual bool checkStatusChange();

	virtual String sendCommand(String json);
};

