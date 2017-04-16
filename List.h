#pragma once
#include "Sensor.h"
#include "ObjectClass.h"

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

