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

#ifdef ESP8266
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif

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
public:
	IRReceiveSensor(int id, uint8_t pin, bool enabled, String address, String name);
	~IRReceiveSensor();
private:
	static String tag;
	static Logger logger;


#ifdef ESP8266


	virtual void getJson(JsonObject& json);
	//virtual String getJSONFields();
	
	// Use turn on the save buffer feature for more complete capture coverage.
	unsigned long startMillis = 0;
	
	String codetype;
	String code;
	String bit;
	String receivedcommanduuid;

	void dumpACInfo(decode_results *results);
	void receive();
	IRrecv* pirrecv;
	decode_results results;  // Somewhere to store the results
	
public:
	const String STATUS_RECEIVINGIRCODE = "receiving";
	const String STATUS_RECEIVONEIRCODE = "receiveone";
	const String STATUS_RESPONSERECEIVEONEIRCODE = "responsereceiveone";
	const String STATUS_RECEIVEDIRCODE = "received";

	

	virtual void init();
	//virtual bool getJSON(JSONObject *jObject);
	virtual bool checkStatusChange();
	bool receiveCommand(String command, int id, String uuid, String json);
#endif
};
#endif
