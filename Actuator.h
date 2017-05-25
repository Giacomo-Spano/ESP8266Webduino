#pragma once
#include "Sensor.h"
#include "Logger.h"

class Actuator :
	public Sensor
{
public:
	static String tag;
	static Logger logger;

	Actuator(int id, uint8_t pin, bool enabled, String address, String name);
	~Actuator();
	virtual void checkStatus();

	virtual String getJSON();
	virtual String sendCommand(String json);
};

