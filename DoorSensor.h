// DoorSensor.h

#ifndef _DOORSENSOR_h
#define _DOORSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual String getJSONFields();
	bool openStatus = false;
	bool testOpenStatus;

public:
	

	DoorSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~DoorSensor();

	virtual void init();	
	virtual bool getJSON(JSONObject *jObject);
	virtual bool checkStatusChange();
	virtual CommandResponse receiveCommand(String json);

	bool getOpenStatus();

};
#endif

