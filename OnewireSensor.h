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


class OnewireSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	OneWire* oneWirePtr;
	DallasTemperature* pDallasSensors;

public:
	//virtual bool getJSON(JSONObject *jObject);
	//virtual void loadChildren(JSONObject json);
	static const int avTempsize = 10;

	//const int checkTemperature_interval = 10000;//60000; // 60 seconds
	//unsigned long lastCheckTemperature = 0;//-flash_interval;

	//OnewireSensor();
	OnewireSensor(int id, uint8_t pin, bool enabled, String address, String name);
	//virtual void loadChildren(JSONArray& jarray);
	virtual void loadChildren(JsonArray& jsonarray);
	~OnewireSensor();
	virtual bool checkStatusChange();

	void beginTemperatureSensors();
	virtual void getJson(JsonObject& json);

	static const int maxTempSensors = 10; // max num sensori onewire sullo stesso pin
	//TemperatureSensorOld temperatureSensors[maxTempSensors];
	int tempSensorNum = 0;
	

	int avTempCounter = 0;
	float avTemp[avTempsize];

	bool readTemperatures();
	//virtual String getJSONFields();
	virtual void init();
	//float getTemperature(int index);
	//float getAvTemperature(int index);
	//void addTemperatureSensorsFromJson(JSON sensorJson);
};

#endif