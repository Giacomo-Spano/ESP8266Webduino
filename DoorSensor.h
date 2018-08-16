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
#include <ArduinoJson.h> 

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual void getJson(JsonObject& json);
	
public:
	const String STATUS_DOOROPEN = "dooropen";
	const String STATUS_DOORCLOSED = "doorclosed";

	DoorSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~DoorSensor();

	virtual void init();	
	virtual bool checkStatusChange();
	bool receiveCommand(String command, int id, String uuid, String json);
};
#endif

