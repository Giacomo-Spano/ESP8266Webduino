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
	virtual String getJSON();
	

	int avTempCounter = 0;
	float avTemp[avTempsize];
	

	void readTemperature(DallasTemperature *pDallasSensors);
	float getTemperature();
	float getAvTemperature();
	
private:
	float temperature;
	float avTemperature;
	
};

