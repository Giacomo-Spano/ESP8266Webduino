#pragma once
#include "Sensor.h"

class Actuator :
	public Sensor
{
public:
	Actuator();
	~Actuator();

	virtual String getJSON();

};

