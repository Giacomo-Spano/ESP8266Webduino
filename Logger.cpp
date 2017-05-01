#include "Logger.h"
#include "Command.h"


extern Logger logger;

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

	if (logFile.size() > 100000) {

		logFile.close();
		
		SPIFFS.remove("/log/log1.txt");
		
		if (SPIFFS.rename(logFileName, "/log/log1.txt")) {
			Serial.println("file : " + logFileName + " renamed \n");
		}

		logFile = SPIFFS.open(logFileName, "w");
		logFile.println("----------------log restarted-----------");
		//logFile.println(txt);
	}
	logFile.println(txt);
	
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

	String date = Logger::getStrDate();
	String header = String(ESP.getFreeHeap()) + " " + date + " " + tag + ": ";
	
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

void Logger::init() {

	Serial.println("\n\n******INIT LOG*******");

	// always use this to "mount" the filesystem
	bool result = SPIFFS.begin();
	Serial.println("SPIFFS opened: " + result);


	/*Dir dir = SPIFFS.openDir("/log");
	while (dir.next()) {

		Serial.println(dir.fileName());
		File f = dir.openFile("r");
		Serial.println(f.size());
	}


	String filename = "/log/log.txt";
	File f = SPIFFS.open(filename, "r");
	if (f) { // file  exixt
		Serial.println("filename: " + filename + " exist\n");
		f.close();

		SPIFFS.remove("/log/log1.txt");
		if (SPIFFS.rename(filename, "/log/log1.txt"))
			Serial.println("file : " + filename + " renamed: \n");
	}*/

	
	Dir dir2 = SPIFFS.openDir("/log");
	while (dir2.next()) {

		Serial.println(dir2.fileName());
		File f = dir2.openFile("r");
		Serial.println(f.size());
	}

	


	logFileName = "/log/log.txt";
	Serial.println("opening " + logFileName);
	// open the file in write mode
	logFile = SPIFFS.open(logFileName, "w+");
	if (!logFile) {
		Serial.println("file open failed - file does not exit");

		logFile = SPIFFS.open(logFileName, "w+");
		if (!logFile) {
			Serial.println("file creation failed");
		}
	}
	else {
		Serial.println("logFile: " + logFileName + " opened\n");

		logFile.seek(logFile.size(), SeekSet);
		// now write two lines in key/value style with  end-of-line characters
		logFile.println(Logger::getStrDate());
		logFile.println("inizioxx");
	}	
	//logFile.close();
}

String Logger::formattedJson(String str)
{

	//char *json = str.c_str();

	String pretty;

	if (str == NULL || str.length() == 0)
	{
		return pretty;
	}

	//String str = String(json);
	bool        quoted = false;
	bool        escaped = false;
	String INDENT = ".";
	int indentval = 1;
	int         indent = 0;
	int         length = (int)str.length();
	int         i;

	for (i = 0; i < length; i++)
	{
		char ch = str[i];

		switch (ch)
		{
		case '{':
		case '[':
			pretty += ch;

			if (!quoted)
			{
				pretty += "\n";

				if (!(str[i + 1] == '}' || str[i + 1] == ']'))
				{
					++indent;

					for (int j = 0; j < indent*indentval; j++)
					{
						pretty += INDENT;
					}
				}
			}

			break;

		case '}':
		case ']':
			if (!quoted)
			{
				if ((i > 0) && (!(str[i - 1] == '{' || str[i - 1] == '[')))
				{
					pretty += "\n";

					--indent;

					for (int j = 0; j < indent*indentval; j++)
					{
						pretty += INDENT;
					}
				}
				else if ((i > 0) && ((str[i - 1] == '[' && ch == ']') || (str[i - 1] == '{' && ch == '}')))
				{
					for (int j = 0; j < indent*indentval; j++)
					{
						pretty += INDENT;
					}
				}
			}

			pretty += ch;

			break;

		case '"':
			pretty += ch;
			escaped = false;

			if (i > 0 && str[i - 1] == '\\')
			{
				escaped = !escaped;
			}

			if (!escaped)
			{
				quoted = !quoted;
			}

			break;

		case ',':
			pretty += ch;

			if (!quoted)
			{
				pretty += "\n";

				for (int j = 0; j < indent*indentval; j++)
				{
					pretty += INDENT;
				}
			}

			break;

		case ':':
			pretty += ch;

			if (!quoted)
			{
				pretty += " ";
			}

			break;

		default:
			pretty += ch;

			break;
		}
	}

	return pretty;
}
