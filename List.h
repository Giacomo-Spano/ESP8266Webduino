#pragma once

#ifdef ESP_DEBUG
#include "Emulator\ESPEmulator.h"
#endif

#include "ObjectClass.h"
#include "Logger.h"


class List
{
private:

	static String tag;
	static Logger logger;

public:
	List();
	~List();
	bool add(ObjectClass* elem);
	void clearAll();
	void init();
	int length();
	ObjectClass* getNext();
	bool hasNext();
	ObjectClass* getFirst();
	ObjectClass* gestLast();
	ObjectClass* get(int n);
	void show();

	ObjectClass* first;
	ObjectClass* current;
	int count = 0;
};
