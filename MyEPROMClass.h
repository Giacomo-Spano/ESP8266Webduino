// MyEPROMClass.h

#ifndef _MYEPROMCLASS_h
#define _MYEPROMCLASS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "JSONObject.h"
#include "Logger.h"

class MyEPROMClass
{
private:
	static String tag;
	static Logger logger;
	const int epromSize = 4096;
	static bool epromInitialized;

public:
	MyEPROMClass();
	~MyEPROMClass();

	void init();

	int readJSON(int index, JSONObject *json);
	int readInt(int index, int *value);
	int readString(int index, String *txt);

	int writeInt(int index, int value);
	int writeJSON(int index, JSONObject *json);
	int writeString(int index, String *txt);

};
#endif

