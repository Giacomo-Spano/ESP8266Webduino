#pragma once
#include "Arduino.h"


class Sensor
{
public:
	Sensor();
	~Sensor();

	virtual String getJSON();
	virtual String getSensorAddress();

	String sensorname = "sensorname";
	uint8_t sensorAddr[8]; // indirizzi sensori

	Sensor* next = NULL;
};

