#pragma once

#include <Arduino.h>
#include "Logger.h"
#include "Shield.h"
//#include "HttpHelper.h"
#include "JSON.h"
#include "Program.h"

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
	bool requestShieldSettings();
	bool requestTime();
	boolean sendSensorStatus(String json);
	boolean sendShieldStatus(String json);
	boolean requestZoneTemperature(String json);
};

