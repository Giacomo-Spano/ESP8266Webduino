#pragma once
#include "Sensor.h"
class List
{
private:
	String tag;
public:
	List();
	~List();
	bool add(Sensor* elem);
	//bool prova(Sensor* elem);
	void init();
	int length();
	Sensor* getNext();
	bool hasNext();
	Sensor* getFirst();
	Sensor* gestLast();
	Sensor* get(int n);
	void show();

	Sensor* first;
	Sensor* current;
	int count = 0;
};

