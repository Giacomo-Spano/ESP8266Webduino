#ifndef _IRSENSOR_h
#define _IRSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"

#ifdef ESP8266
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#endif

class IRSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

#ifdef ESP8266
	IRsend *pirsend;
	IRDaikinESP *pdaikinir;
#endif
	bool sendCode(JsonObject& json);

	bool sendDaikin();
	bool sendSamsungTv(int code);
	bool sendRobotClean();
	bool sendRobotHome();
	bool sendHarmanKardonDisc();

public:
	
	IRSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~IRSensor();

	virtual void init();
	virtual bool checkStatusChange();
	virtual bool receiveCommand(String command, int actuatorId, String uuid, String json);

};
#endif
