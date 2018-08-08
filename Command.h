#pragma once

#include <Arduino.h>
#include "Logger.h"
#include "Shield.h"
#include "JSON.h"

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
	bool requestShieldSettings(String MACAddress);
	bool requestTime(String macAddress);
	boolean sendSensorStatus(String json);
	boolean sendShieldStatus(String json);
	boolean requestZoneTemperature(int id, String json);
};

