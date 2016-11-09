#pragma once

#include <Arduino.h>
#include <Time.h>
#include "TimeLib.h"
#include "Logger.h"
#include "Settings.h"
#include "HttpHelper.h"
#include "OneWireSensors.h"
#include "JSON.h"
#include "Program.h"


class Command
{
private:
	String tag;
public:
	const String HeaterActuatorSubaddress = "01";
	static const int maxLogSize = 1000;
	//time_t getNtpTime();
	void digitalClockDisplay();
	void printDigits(int digits);

	static String servername;
	static int serverport;

	Command();
	void setServer(String servername, int serverport);
	~Command();

	int registerShield(Settings settings, OneWireSensors ows);
	boolean sendActuatorStatus(Settings settings, OneWireSensors ows, Program programSettings);
	boolean sendSensorsStatus(Settings settings, OneWireSensors ows);
	bool sendLog(String log, int shieldid, String servername, int port);
};

