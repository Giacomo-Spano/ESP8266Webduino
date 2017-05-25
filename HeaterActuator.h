#pragma once
#include "Actuator.h"
#include "Program.h"

class HeaterActuator :
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
	static char* statusStr[];// = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };
		
	int manualMode;
	//String subaddress = "HeaterActuator-";
	const bool RELE_ON = HIGH;//LOW
	const bool RELE_OFF = LOW;//LOW
	static int const sensor_local = 0;
	unsigned long last_RemoteSensor = 0;
	unsigned long remoteSensorTimeout = 5* 60 * 1000; // tempo dopo il quale il programa si disattiva

public:
	const char* command_ProgramOff = "programoff";
	const char* command_ProgramOn = "programon";
	const char* command_disabled = "disabled";
	const char* command_enabled = "enabled";
	const char* command_ManualOff = "manualoff";
	const char* command_Manual = "manual";
	const char* command_ManualEnd = "manualend";

	static const int MANUALMODE_DISABLED = 0;  // modalità manuale disabilitata
	static const int MANUALMODE_AUTO = 1;  // temperatura automatica >= alk sensorid
	static const int MANUALMODE_OFF = 2; // sempre spento
	static const int MANUALMODE_END = 3; // sempre spento

	HeaterActuator(int id, uint8_t pin, bool enabled, String address, String name);
	//HeaterActuator();
	~HeaterActuator();
	void updateReleStatus();
	//virtual String getJSON() override;
	virtual CommandResponse receiveCommand(String json) override;
	void setStatus(int status);
	int getStatus();
	String getStatusName();
	void setReleStatus(int status);
	int getReleStatus();
	void enableRele(boolean on);
	void setRemote(float temp);
	bool statusChanged();
	bool releStatusChanged();
	void saveOldReleStatus();
	void saveOldStatus();
	void setTargetTemperature(float target);
	float getTargetTemperature();	
	float getRemoteTemperature();
	float getLocalTemperature();
	void setSensorRemote(bool enable, int sensorId);
	int getRemoteSensorId();
	bool sensorIsRemote();
	int getLocalSensorId();
	bool programEnded();
	time_t getRemaininTime();
	time_t getProgramDuration();
	int getActiveProgram();
	int getActiveTimeRange();
	void setLocalTemperature(float temperature);
	void changeProgram(String command, long duration, bool sensorRemote, float remotetemperature, int sensorId, float target, int program, int timerange);
	virtual String getSensorAddress();

	time_t programStartTime = 0;
	time_t programDuration = 30000;

	Program programSettings;
private:
	int currentStatus = Program::STATUS_IDLE;
	int oldCurrentStatus = currentStatus;
	bool releStatus = false;
	bool oldReleStatus = false;
	float targetTemperature = 0.0;
	float remoteTemperature = 0;
	int remoteSensorId = 0; 
	int localSensorId = 0;
	bool sensorRemote = false;//indica se il sensore è locale o remoto
	int activeProgram = -1;
	int activeTimerange = -1;
	unsigned long totalConsumptionTime = 0;
	unsigned long lastConsumptionEnableTime = 0;
	unsigned long ConsumptionStartTime;
	float localAvTemperature = 0;
	float oldLocalAvTemperature = 0;	
};


