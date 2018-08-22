#pragma once

#include "SoftwareSerial.h"

extern SoftwareSerial swSer1;
extern SoftwareSerial nextionSoftwareSerial;


#include "Nextion.h"

class NextionDisplay
{
private:
	static String tag;
	static Logger logger;

public:
	NextionDisplay();
	~NextionDisplay();

	void init();
	void loop();

	
};

