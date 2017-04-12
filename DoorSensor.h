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
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Logger.h"

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

public:
	DoorSensor();
	~DoorSensor();
	virtual String getJSONFields();
	virtual void init();
	bool getOpenStatus();
	bool checkDoorStatus();
		
private:
	bool openStatus = false;
};
#endif

