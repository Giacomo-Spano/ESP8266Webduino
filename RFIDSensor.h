#ifndef _RFIDSENSOR_h
#define _RFIDSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D8
#define RST_PIN D3
//MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


class RFIDSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual String getJSONFields();
	bool openStatus = false;
	bool testOpenStatus;
	MFRC522* mfrc522;   // Create MFRC522 instance.
	String lastcard;
	unsigned long lastCardRead;
	unsigned long cardRead_interval;

public:


	RFIDSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~RFIDSensor();

	virtual void init();
	virtual bool getJSON(JSONObject *jObject);
	virtual bool checkStatusChange();
	//virtual CommandResponse receiveCommand(String json);

	//bool getOpenStatus();

};

#endif
