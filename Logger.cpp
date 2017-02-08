#include "Logger.h"
#include "Command.h"

//#include <Time.h>
//#include "TimeLib.h"

extern Logger logger;

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

	if (!truncated) {

		if (toBeSent.length() > maxLogbuffer) {
			
			/*if (txt.length() < maxLogbuffer)
				toBeSent = toBeSent.substring(txt.length()) + txt.length();
			else
				toBeSent = "";*/

			toBeSent += txt + "\n\n--- TRUNCATED ---\n\n";
			truncated = true;
		}
		else {
			toBeSent += txt;
		}
	}
	//line = "";
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
	print(tag,str);
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

int packetcounter = 0;

bool Logger::send() {

	//String temp = "**FINE*";
	if (toBeSent.length() > Command::maxLogSize) {
		Command command;

		Serial.println("SEND LOG");
		//Serial.print("toBeSent = ");
		//Serial.println(toBeSent);

		String substr = toBeSent.substring(0, Command::maxLogSize-1);
		
		//Serial.print("SEND LOG PACKET ");
		//Serial.print(packetcounter++,DEC);
		//Serial.print(":");
		//Serial.println(substr);
		bool res = command.sendLog(substr/*, shieldid, serverName, port*/);
		if (res) {
			truncated = false;
			Serial.println("PACKET SENT");
			toBeSent = toBeSent.substring(substr.length()/*-1*/);
			//Serial.print("toBeSent=");
			//Serial.println(toBeSent);
		}
		else {
			Serial.println("PACKET NOT SENT");

		}
	}	
}

String Logger::getHeader(String tag) {

	String date = getStrDate();
	String header = String(ESP.getFreeHeap()) + " " + date + " " + tag + ": ";
	
	return header;
}

String Logger::getStrDate() {

	char buffer[50];
	sprintf(buffer, "%02d-%02d-%02d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
	String date = String(buffer);
	return date;
}
