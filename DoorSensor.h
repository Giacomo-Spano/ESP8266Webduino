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

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

public:
	DoorSensor(uint8_t pin, bool enabled, String address, String name);
	~DoorSensor();
	virtual String getJSONFields();
	virtual void init();	
	virtual bool checkStatusChange();

	bool getOpenStatus();

private:
	bool openStatus = false;
};
#endif

