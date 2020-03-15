// HEATERSENSOR.h

#ifndef _HEATERSENSOR_h
#define _HEATERSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"
#include <ArduinoJson.h>

class HeaterSensor :
	public Sensor
{
private:
	//virtual String getJSONFields();


public:
	//virtual bool getJSON(JSONObject * jObject);
	static String tag;
	static Logger logger;

	virtual void init();
	virtual bool checkStatusChange();


private:
	//static char* statusStr[];// = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };
		
	int manualMode;
	const bool RELE_ON = LOW;//HIGH
	const bool RELE_OFF = HIGH;//LOW
	
	unsigned long last_RemoteSensor = 0;
	unsigned long programUpdateTimeout = 5* 60 * 1000; // tempo dopo il quale il programa si disattiva

	unsigned long remoteTemperatureTimeout = 3 * 60 * 1000; // tempo dopo il quale il programa si disattiva
	unsigned long remoteTemperatureRefreshTimeout = 1 * 30 * 1000; // tempo dopo il quale il programa si disattiva

public:
	const char* command_KeepTemperature = "keeptemperature";
	const char* command_Stop_KeepTemperature = "stopkeeptemperature";
	const char* command_Off = "off";
	const char* command_sendTemperature = "sendtemperature";
	const char* command_Idle = "idle";
	//const char* command_Manual = "manual";
	
	//const String STATUS_DISABLED = "disabled"
	const String STATUS_OFF = "off";
	const String STATUS_KEEPTEMPERATURE = "keeptemperature";
	const String STATUS_KEEPTEMPERATURE_RELEOFF = "keeptemperaturereleoff";
	//const String STATUS_MANUAL = "manual";

	HeaterSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~HeaterSensor();
	
	void updateReleStatus();
	bool changeStatus(String command, long duration, float remotetemperature, float target, int actionid, String commanddate, String enddate, int zoneid, int zonesensorid);
	
	//CommandResponse receiveCommand(int actuatorId, String uuid, String json);
	bool receiveCommand(String command, int id, String uuid, String json);

	virtual void getJson(JsonObject& json);

	void setStatus(String status);
	String getStatus();
	void setZone(int _zoneid, int _zonesensorid);
	
	void setReleStatus(bool status);
	bool getReleStatus();
	void enableRele(bool on);
	void setRemoteTemperature(float temp);
	bool statusChanged();
	bool releStatusChanged();
	void saveOldReleStatus();
	//void saveOldStatus();
	void setTargetTemperature(float target);
	float getTargetTemperature();	
	float getRemoteTemperature();
	bool programEnded();
	//bool remoteTemperatureUpdate();
	//bool requestZoneTemperatureUpdate(int shieldid);
	time_t getRemaininTime();
	
	time_t programStartTime = 0;
	time_t programDuration = 3; // durata programma espressa in secondi

	//Program programSettings;
private:
	//String status = STATUS_OFF;
	String oldStatus = STATUS_OFF;
	
	bool releStatus = false;
	bool oldReleStatus = false;
	float targetTemperature = 0.0;
	float remoteTemperature = 0;
	int zoneid = 0; 
	int zonesensorid = 0;
	int activescenario = -1;

	int remainingSeconds = 0;
	int oldRemainingSeconds = 0;


	int activeTimeInterval = -1;
	int activeActionId = -1;
	String lastCommandDate;
	String lastTemperatureUpdate;
	String endDate;
	unsigned long totalConsumptionTime = 0;
	unsigned long lastConsumptionEnableTime = 0;
	unsigned long ConsumptionStartTime;
};

#endif
