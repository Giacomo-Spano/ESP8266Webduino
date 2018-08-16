#pragma once
#include <Arduino.h>
#include <Time.h>
#include "TimeLib.h"
#include "FS.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


class Logger
{
private :
	
	static String tag;
	
	String getHeader(String tag);
	static const int maxLogbuffer = 10000;
	bool truncated = false;
	static String logFileName;
	static File logFile;

	//String tag;
	String line = "";
	static String toBeSent;
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
	void printFreeMem(String tag, String txt);
	void printJson(JsonObject& json);
		
	static String getStrDate();
	static String getStrDayDate();
	static String getStrTimeDate();
	static void init();
	//static void sendLogToServer();
	static String boolToString(bool res);

	//String formattedJson(String str);
};

