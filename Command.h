#pragma once

#include <Arduino.h>

#include "Logger.h"
#include "Shield.h"
#include "HttpHelper.h"
//#include "OneWireSensors.h"
#include "JSON.h"
#include "Program.h"


class Command
{
private:
	String tag;
	//const String StatusUpdate = "statusupdate";
	//const String StatusUpdate = "releupdate";
public:
	
	static const int maxLogSize = 1000;
	//
	//void digitalClockDisplay();
	//void printDigits(int digits);

	static String servername;
	static int serverport;

	Command();
	void setServer(String servername, int serverport);
	~Command();

	int registerShield(Shield shield);
	void sendRestartNotification();
	boolean sendSensorsStatus(Shield shield);
	bool sendLog(String log/*, int shieldid/*, String servername, int port*/);
	int timeSync(/*String servername, int port*/);
	boolean sendActuatorStatus(HeaterActuator actuator);
	boolean download(String filename, Shield shield);

	//time_t getNtpTime();
	static Command *getTimeObject;

	static time_t globalGetNTPTime() {
		Serial.println("AAA");
		return Command::getTimeObject->getNtpTime();
	}
private:
	static time_t getNtpTime();
};

