#include "Logger.h"
#include "Command.h"


extern Logger logger;
//extern bool _mqtt_publish(char* topic, char* payload);
//#define maxjournalcount 100
//String journal[maxjournalcount];
//int journalcount = 0;

String Logger::tag = "Logger";
String Logger::logFileName = "/log/log.txt";
String Logger::toBeSent = "";
File Logger::logFile;

Logger::Logger()
{
}

Logger::~Logger()
{
}

void Logger::printJson(JsonObject& json) {
	json.printTo(Serial);
}

void Logger::println(String tag, String txt) {
	txt = String("\n") + getHeader(tag) + txt;
	print(tag, txt);
}

void Logger::print(String tag, String txt) {
	Serial.print(txt);
}

void Logger::printFreeMem(String tag, String txt) {
	
	String str = "\n\t --Free mem" + String(ESP.getFreeHeap()) + " " + tag +" " + txt + "";
	Serial.print(str);
}


/*void Logger::print(String tag, String txt) {

	if (line.equals("") && txt != NULL && !txt.equals("")) {
		line += getHeader(tag);
	}
	Serial.print(txt);
	line += txt;
}*/

void Logger::print(String tag, char* buffer) {
	String str = String(buffer);
	print(tag, str);
}

void Logger::println(String tag, char* buffer) {
	String str = String(buffer);
	println(tag, str);
}

void Logger::print(String tag, int val) {
	String str = String(val);
	print(tag, str);
}

void Logger::println(String tag, int val) {
	String str = String(val);
	println(tag, str);
}

String Logger::getHeader(String tag) {

	String date = Logger::getStrDate();
	String header = String(date + " " + tag + "(" + ESP.getFreeHeap()) + "): ";
	return header;
}

String Logger::getStrDate() {

	char buffer[50];
	sprintf(buffer, "%02d-%02d-%02d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
	String date = String(buffer);
	return date;
}

String Logger::getStrDayDate() {

	char buffer[50];
	sprintf(buffer, "%02d-%02d-%02d", day(), month(), year());
	String date = String(buffer);
	return date;
}

String Logger::getStrTimeDate() {

	char buffer[50];
	sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
	String date = String(buffer);
	return date;
}

String Logger::boolToString(bool val) {
	String str = F("true");
	if (!val)
		str = F("false");
	return str;
}

void Logger::init() {

	
}

