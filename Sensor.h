#pragma once
#include "Arduino.h"


class Sensor
{
public:
	Sensor();
	~Sensor();

	String getJSON();
	String getSensorAddress();

	String sensorname = "sensorname";
	uint8_t sensorAddr[8]; // indirizzi sensori

	Sensor* next = NULL;
};

