#pragma once
#include "Sensor.h"

class Actuator :
	public Sensor
{
public:
	Actuator();
	~Actuator();
	virtual void checkStatus();

	virtual String getJSON();
	virtual String sendCommand(String json);

};

