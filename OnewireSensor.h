// OnewireSensor.h

#ifndef _ONEWIRESENSOR_h
#define _ONEWIRESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Logger.h"
#include "JSON.h"

class TemperatureSensor {

public:
	String name = "Sensor name";
	int id = 0;
	float temperature;
	float avTemperature;
	uint8_t sensorAddr[8]; // indirizzi fisico sensore

	String getPhisicalAddress() {

		String str = "";
		for (int i = 0; i < 8; i++) {

			char temp[30];
			sprintf(temp, "%02X", sensorAddr[i]);
			str += String(temp);

			if (i < 7) {
				str += ":";
			}
		}
		return str;
	}
};

class OnewireSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	OneWire* oneWirePtr;
	DallasTemperature* pDallasSensors;

public:
	static const int avTempsize = 10;

	const int checkTemperature_interval = 60000; // 60 seconds
	unsigned long lastCheckTemperature = 0;//-flash_interval;

	OnewireSensor();
	~OnewireSensor();

	static const int maxTempSensors = 10; // max num sensori onewire sullo stesso pin
	TemperatureSensor temperatureSensors[maxTempSensors];
	int tempSensorNum = 0;

	int avTempCounter = 0;
	float avTemp[avTempsize];

	bool readTemperatures();
	virtual String getJSONFields();
	virtual void init();
	float getTemperature(int index);
	float getAvTemperature(int index);
	void addTemperatureSensorsFromJson(JSON sensorJson);
};

#endif