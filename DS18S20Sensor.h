#pragma once

#include "Sensor.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class DS18S20Sensor :
	public Sensor
{
private:
	String tag;

public:
	static const int avTempsize = 10;
	static const int sensornamelen = 30;

	DS18S20Sensor();
	~DS18S20Sensor();

	int avTempCounter = 0;
	float avTemp[avTempsize];
	float temperature;
	float avTemperature;

	void readTemperature();
	
private:
	void readAverageTemperature();
	//float getTemp();
	
};

