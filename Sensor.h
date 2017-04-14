#pragma once
#include "Arduino.h"
#include "Logger.h"

class Sensor
{
private:
	static String tag;
	static Logger logger;

public:
	Sensor(uint8_t pin, bool enabled, String address, String name);
	//Sensor();
	~Sensor(); 

	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	String address;

	int checkStatus_interval;// = 10000;//60000; // 60 seconds
	unsigned long lastCheckStatus;// = 0;//-flash_interval;

	String getJSON();
	static const int sensorNameLen = 20;
	static const int sensorAddressLen = 20;
	static const int sensorTypeLen = 20;

	virtual String getJSONFields();
	virtual void init();
	virtual bool checkStatusChange();
	//String getSensorAddress();
		
	Sensor* next = NULL;
};

