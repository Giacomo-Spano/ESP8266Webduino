#ifndef _IRRECEIVESENSOR_h
#define _IRRECEIVESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include "CommandResponse.h"
#include <IRremoteESP8266.h>


#include <IRrecv.h>
#include <IRutils.h>
#if DECODE_AC
#include <ir_Daikin.h>
#include <ir_Fujitsu.h>
#include <ir_Gree.h>
#include <ir_Haier.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Toshiba.h>
#endif  // DECODE_AC

class IRReceiveSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual String getJSONFields();
	
	// Use turn on the save buffer feature for more complete capture coverage.
	IRrecv *pirrecv;
	decode_results results;  // Somewhere to store the results
	void dumpACInfo(decode_results *results);
	void receive();
	
public:
	//const String STATUS_DOOROPEN = "dooropen";
	//const String STATUS_DOORCLOSED = "doorclosed";

	IRReceiveSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~IRReceiveSensor();

	virtual void init();
	//virtual bool getJSON(JSONObject *jObject);
	virtual bool checkStatusChange();
	bool receiveCommand(String command, int id, String uuid, String json);
	};
#endif
