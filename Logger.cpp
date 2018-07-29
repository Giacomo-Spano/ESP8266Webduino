#include "Logger.h"
#include "Command.h"


extern Logger logger;
extern bool _mqtt_publish(char* topic, char* payload);


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
	
	String str = "\nFree mem" + String(ESP.getFreeHeap()) + " " + tag +" " + txt + "\n";
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

int packetcounter = 0;

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

void Logger::sendLogToServer() {
#ifdef ESP8266
	Serial.println("\n >>Logger::sendLogToServer\n\n\n");
	/*String str = "prova invio testo";
	HttpHelper hplr;
	Shield shield;
	String postResult;
	boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/log", str, &postResult);*/


	String fileName = "/log/log.txt";
	Serial.println("\nopening " + fileName);
	// open the file in read mode
	File file = SPIFFS.open(logFileName, "r");
	if (!file) {
		Serial.println("\nfile open failed - file does not exit");
	}
	else {
		String topic = "toServer/shield/log" + String(Shield::getShieldId());
		int count = 0;
		char stopic[50];
		for (int i = 0; i < topic.length(); i++) {
			stopic[count++] = topic.charAt(i);
		}
		stopic[count++] = '\0';
		
		const int size = 256;
		char payload[size];
		Serial.println("logFile: " + fileName + " opened\n");
		int len = 0;
		
		String str;
		while (file.available()) {
			str = file.readStringUntil('\n');
			Serial.print(str);
			//payload[size - 1] = '\0';
			//bool res = _mqtt_publish(stopic, payload);
		}	

		Serial.println("\n <<Logger::sendLogToServer\n\n\n");
	}
#endif
}



void Logger::init() {

	Serial.println("\n\n******INIT LOG*******");

#ifdef ESP8266
	// always use this to "mount" the filesystem
	bool result = SPIFFS.begin();
	Serial.println("SPIFFS opened: " + result);
	

	/*Dir dir2 = SPIFFS.openDir("/log");
	Serial.println("dir /log");
	while (dir2.next()) {

		Serial.print(dir2.fileName());
		Serial.print(" size ");
		File f = dir2.openFile("r");
		Serial.print(f.size());
		Serial.println(" bytes");
		f.close();
	}*/


	logFileName = "/log/log.txt";
	Serial.println("opening " + logFileName);
	// open the file in write mode
	logFile = SPIFFS.open(logFileName, "a+");
	if (!logFile) {
		Serial.println("file open failed - file does not exit");

		/*logFile = SPIFFS.open(logFileName, "w+");
		if (!logFile) {
		Serial.println("file creation failed");
		}*/
	}
	else {
		Serial.println("logFile: " + logFileName + " opened\n");

		//logFile.seek(logFile.size(), SeekSet);
		logFile.seek(0, SeekEnd);
		// now write two lines in key/value style with  end-of-line characters
		logFile.println(Logger::getStrDate());
		int l = logFile.println("inizioxx00");
		Serial.println("written: " + String(l));
		logFile.close();
		//logFile.readBytes
	}
	//logFile.close();
#endif

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
