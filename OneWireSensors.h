#pragma once

//#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//#ifndef _WOL_h
//#define _WOL_h




class OneWireSensors
{

	static const int maxSensorNumber = 10; // numero massimo di sensori collegabili sun bus onwwire
	static const int avTempsize = 10;
	static const int sensornamelen = 30;
	
public:
	OneWireSensors();
	~OneWireSensors();

	// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
	void discoverOneWireDevices(void);
	float readTemperatures();
	float getAverageTemperature(int n);
	String getSensorAddress(int n);

	int sensorCount = 0; // numero di sensori collegati
	char sensorname[maxSensorNumber][sensornamelen] = { "sensor1", "sensor2", "sensor3", "sensor4", "sensor5", "sensor6", "sensor7", "sensor8", "sensor9", "sensor10", };
	byte sensorAddr[maxSensorNumber][8]; // indirizzi sensori
	int avTempCounter = avTempsize;
	float avTemp[maxSensorNumber][avTempsize];
	float sensorTemperatures[maxSensorNumber];
	float sensorAvTemperatures[maxSensorNumber];


private:
	

	
	

	
};

//#endif