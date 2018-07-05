// HORNSensor.h

#ifndef _HORNSENSOR_h
#define _HORNSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"

class HornSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual String getJSONFields();
	bool alarmActive = false;
	bool testOpenStatus = false;

public:


	HornSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~HornSensor();

	virtual void init();
	virtual bool getJSON(JSONObject *jObject);
	virtual bool checkStatusChange();
	bool receiveCommand(String command, int id, String uuid, String json);

	bool getAlarmActiveStatus();

};
#endif

