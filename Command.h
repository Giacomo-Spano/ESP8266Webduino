#pragma once

#include <Arduino.h>
#include "Logger.h"
#include "Shield.h"

class Command
{
private:
	static String tag;
	static Logger logger;

public:	
	static const int maxLogSize = 1000;
	static String serverName;
	static int serverport;

	Command();
	~Command();
	bool requestShieldSettings(String MACAddress, String rebootreason);
	bool requestTime(String macAddress);
	boolean sendSensorStatus(JsonObject& json);
	boolean sendShieldStatus(String json);
	boolean requestZoneTemperature(int id, String json);
};

