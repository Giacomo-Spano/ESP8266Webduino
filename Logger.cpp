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

/*void Logger::println(String tag, String txt) {

	//if (line.equals("")) {
	if (line.length() == 0 && txt != NULL && !txt.equals("")) {
		line += getHeader(tag);
	}
	line += txt;
	Serial.println(line);

	if (!truncated) {

		if (toBeSent.length() > maxLogbuffer) {
			toBeSent += line + "\n\n--- TRUNCATED ---\n\n";
			truncated = true;
		}
		else {
			toBeSent += line + "\n";
		}
	}
	line = "";
}*/

void Logger::println(String tag, String txt) {
	txt = String("\n") + getHeader(tag) + txt;
	print(tag, txt);
}

void Logger::print(String tag, String txt) {

	/*if (txt != NULL && !txt.equals("")) {
		txt = getHeader(tag) + txt;
	}*/
	//line += txt;
	Serial.print(txt);

	/*journal[journalcount] = txt;
	if (journalcount < maxjournalcount)
		journalcount++;
	else
		journalcount = 0;*/

	/*if (logFile.size() > 100000) {

		logFile.close();

		SPIFFS.remove("/log/log1.txt");

		if (SPIFFS.rename(logFileName, "/log/log2.txt")) {
			Serial.println("file : " + logFileName + " renamed \n");
		}

		logFile = SPIFFS.open("/log/log.txt", "a+");
		logFile.println("----------------log restarted-----------");
		//logFile.println(txt);
	}*/
	//logFile.println(txt);


	//logFileName = "/log/log.txt";

	//logFile = SPIFFS.open(logFileName, "a+");
	if (!logFile) {
		//Serial.println("file open failed - file does not exit");

		/*logFile = SPIFFS.open(logFileName, "w+");
		if (!logFile) {
		Serial.println("file creation failed");
		}*/
	}
	else {
		//Serial.println("logFile: " + logFileName + " opened\n");

		//logFile.seek(0,SeekEnd);
		//int l = logFile.println(txt);
		//Serial.println("written: " + String(l));
		//logFile.close();
		//logFile.readBytes
	}

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
	String str = "true";
	if (val)
		str = "false";
	return str;
}

void Logger::init() {

	
}

