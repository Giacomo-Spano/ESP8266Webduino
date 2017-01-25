#pragma once
#include "Actuator.h"
#include "Program.h"

class HeaterActuator :
	public Actuator
{
	
private:
	String tag;
	int manualMode;
	String subaddress = "HeaterActuator-";
	int relePin = D5; // rel? pin
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

	HeaterActuator();
	~HeaterActuator();
	void updateReleStatus();
	virtual String getJSON() override;
	virtual void checkStatus() override;
	virtual String sendCommand(String json) override;
	void setStatus(int status);
	int getStatus();
	void setReleStatus(int status);
	int getReleStatus();
	void enableRele(boolean on);
	int getRelePin();
	void setRelePin(int pin);
	bool statusChanged();
	bool releStatusChanged();
	void saveOldReleStatus();
	void saveOldStatus();
	void init(String MACAddress);
	void setTargetTemperature(float target);
	float getTargetTemperature();
	void setRemoteTemperature(float remote);
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


