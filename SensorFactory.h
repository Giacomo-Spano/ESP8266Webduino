#ifndef _SENSORFACTORY_h
#define _SENSORFACTORY_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "JSONObject.h"

class SensorFactory
{
private:
	static String tag;
	static Logger logger;
public:
	SensorFactory();
	~SensorFactory();

	static Sensor* createSensor(String type, uint8_t pin, bool enabled, String address, String name);
	static Sensor * createSensor(JSONObject* json);
};

#endif

