#pragma once
#include "Sensor.h"
#include "Logger.h"

class Actuator :
	public Sensor
{
public:
	static String tag;
	static Logger logger;

	Actuator();
	~Actuator();
	virtual void checkStatus();

	virtual String getJSON();
	virtual String sendCommand(String json);
};

