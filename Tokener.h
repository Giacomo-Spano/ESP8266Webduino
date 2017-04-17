#pragma once

#ifdef ESP_DEBUG
#include "Emulator\ESPEmulator.h"
#endif

#include "Logger.h"

class Tokener
{
private:
	static String tag;
	static Logger logger;
public:
	Tokener(String json);
	~Tokener();

	String json;
	char next();
	char nextClean();
	String nextTo(char toChar);
	bool nextIsFloat();
	float nextFloat();
	int nextInteger();
	bool nextBool();

	String nextArray();
	String nextJson();

protected:
	int current = 0;

};

