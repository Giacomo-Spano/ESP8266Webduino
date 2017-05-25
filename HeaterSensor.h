#pragma once
#include "Actuator.h"
#include "Program.h"
#include "CommandResponse.h"

class HeaterSensor :
	public Sensor
{
private:
	virtual String getJSONFields();


public:
	virtual bool getJSON(JSONObject * jObject);
	static String tag;
	static Logger logger;

	virtual void init();
	virtual bool checkStatusChange();


private:
	//static char* statusStr[];// = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };
		
	int manualMode;
	const bool RELE_ON = HIGH;//LOW
	const bool RELE_OFF = LOW;//LOW
	
	unsigned long last_RemoteSensor = 0;
	unsigned long remoteSensorTimeout = 5* 60 * 1000; // tempo dopo il quale il programa si disattiva

public:
	const char* command_KeepTemperature = "keeptemperature";
	const char* command_Off = "off";
	const char* command_sendTempareture = "sendtemperature";
	
	//const String STATUS_DISABLED = "disabled"
	const String STATUS_OFF = "off";
	const String STATUS_KEEPTEMPERATURE = "keeptemperature";

	HeaterSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~HeaterSensor();
	
	void updateReleStatus();
	bool changeStatus(String command, long duration, float remotetemperature, float target, int scenario, int timeinterval, int zone);
	
	CommandResponse receiveCommand(String json);


	void setStatus(String status);
	String getStatus();
	
	void setReleStatus(int status);
	int getReleStatus();
	void enableRele(boolean on);
	void setRemoteTemperature(float temp);
	bool statusChanged();
	bool releStatusChanged();
	void saveOldReleStatus();
	void saveOldStatus();
	void setTargetTemperature(float target);
	float getTargetTemperature();	
	float getRemoteTemperature();
	bool programEnded();
	time_t getRemaininTime();
	
	time_t programStartTime = 0;
	time_t programDuration = 30000;

	//Program programSettings;
private:
	String status = STATUS_OFF;
	String oldStatus = status;

	bool releStatus = false;
	bool oldReleStatus = false;
	float targetTemperature = 0.0;
	float remoteTemperature = 0;
	int zoneId = 0; 
	int activescenario = -1;
	int activeTimeInterval = -1;
	unsigned long totalConsumptionTime = 0;
	unsigned long lastConsumptionEnableTime = 0;
	unsigned long ConsumptionStartTime;
};


