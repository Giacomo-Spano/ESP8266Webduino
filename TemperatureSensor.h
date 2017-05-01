// TemperatureSensor.h

#ifndef _TEMPERATURESENSOR_h
#define _TEMPERATURESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"



class TemperatureSensor : public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual String getJSONFields(int jsontype);


public:
	virtual JSONObject getJSON2();
	TemperatureSensor(uint8_t pin, bool enabled, String address, String name);
	~TemperatureSensor();
	//virtual String getJSONFields();
	virtual void init();
	virtual bool checkStatusChange();

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

#endif