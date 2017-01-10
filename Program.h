#pragma once
#include <Arduino.h>

class Program
{
public:

	static const int STATUS_IDLE = 1;
	static const int STATUS_PROGRAMACTIVE = 2;
	static const int STATUS_MANUAL_AUTO = 3;
	static const int STATUS_DISABLED = 4;
	static const int STATUS_RESTARTED = 5;
	static const int STATUS_MANUAL_OFF = 6;

	

	Program();
	~Program();

	/*time_t programStartTime = 0;
	time_t programDuration = 30000;*/

	/*int currentStatus = STATUS_IDLE;
	int oldcurrentstatus = currentStatus;*/

	//bool releStatus = false;
	//bool oldReleStatus = false;
	//int actuatorAddress = 0;
};

