#pragma once

#ifdef ESP_DEBUG
#include "Emulator\ESPEmulator.h"
#endif

#include "ObjectClass.h"
#include "List.h"


class JSONArrayObject 
{
private:
	static String tag;
	static Logger logger;

public:
	JSONArrayObject(String str);
	~JSONArrayObject();

	List jsonArray;

	void parse(String str);
};

