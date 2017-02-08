#pragma once
#include <Arduino.h>
#include <Time.h>
#include "TimeLib.h"


class Logger
{
private :
	
	
	String getHeader(String tag);
	static const int maxLogbuffer = 10000;
	bool truncated = false;

	//String tag;
	String line = "";
	String toBeSent = "";
public:
	Logger();
	//Logger(String tag);
	//void setTag(String tag);
	~Logger();

	void println(String tag,String txt);
	void print(String tag, String txt);
	void print(String tag, char* buffer);
	void println(String tag, char* buffer);
	void print(String tag, int val);
	void println(String tag, int val);
	
	bool send(/*int shieldid, String serverName, int port*/);
	
	String getStrDate();
	
};

