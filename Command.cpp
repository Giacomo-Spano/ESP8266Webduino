#include "Command.h"
#include "Program.h"
#include "Util.h"

extern Logger logger;

extern const char* statusStr[];
char* boolStr[] = { "false", "true" };

Command::Command()
{
	tag = "Command";
}

void Command::setServer(String servername, int serverport)
{
	//Command::servername = servername;
	//Command::serverport = serverport;
}


Command::~Command()
{
}


time_t serverTime;

time_t getNtpTime() {
	tmElements_t tm;
	tm.Day = 6;
	tm.Month = 11;
	tm.Year = 2016;
	tm.Second = 0;
	tm.Minute = 0;
	tm.Hour = 0;
	time_t t = makeTime(tm);

	return serverTime;
}

int packetcount = 1;

bool Command::sendLog(String log, int shieldid, String servername, int port)
{
	//Serial.println(F("SENDLOG"));
	if (log.length() > Command::maxLogSize)
		return false;

	String packetend = "#END#";
	String packetstart = "#START#";
	String packetnumber = String(packetcount++);
	String strid = String(shieldid);
	if (packetcount > 99999) 
		packetcount = 0;
	/*for (int i = packetnumber.length(); i < 5; i++)
	{
		//packetend += " ";
		packetstart += " ";
	}*/

	/*for (int i = strid.length(); i < 5; i++)
	{
		strid = "x"+ strid;
	}*/
	//strid += "y";	
	//packetstart += ":" + packetnumber + ":" + strid + ":";
	//packetend += packetnumber;

	//const int terminatorsize = 24;//6+5 + 8+5;//int l = packetend.length();

	//char   buffer[maxLogSize + terminatorsize];

	//String json = packetstart + log + packetend;
	/*String json = "{\"shieldid\":\"" + String(shieldid);
	json += "\",\"#\":\"" + String(packetnumber);
	json += "\",\"log\":\"" + log;
	json += "\"}";*/

	String json = packetstart + ":" + packetnumber + ":" + strid + ":" + log + ":" + packetend;
		
	HttpHelper hplr;
	String result;
	//boolean res = hplr.post(servername, port, "/webduino/log", buffer, sizeof(buffer), &result);
	boolean res = hplr.post(servername, port, "/webduino/log", json, &result);
	
	//Serial.print("answer = ");
	//Serial.println(result);

	JSON jsonResult(result);
	String resultvalue = jsonResult.jsonGetString("result");
	
	if (resultvalue.equalsIgnoreCase("success")) {

		return true;
	}
	else {
		return false;
	}
}

int Command::registerShield(Settings settings, OneWireSensors ows)
{
	logger.println(tag,F("REGISTER SHIELD"));

	const int buffersize = 500;
	char   buffer[buffersize];
	String str = "{";
	str += "\"MAC\":\"" + String(settings.MAC_char) + "\"";
	str += ",\"boardname\":\"" + String(settings.boardname) + "\"";
	str += ",\"localIP\":\"" + settings.localIP + "\"";
	str += ",\"localPort\":\"" + String(settings.localPort) + "\"";
	str += ",\"sensors\":[";
	for (int i = 0; i < ows.sensorCount; i++) {

		if (i != 0)
			str += ",";
		str += "{\"name\":\"";
		str += String(ows.sensorname[i]) + "\"";
		str += ",\"type\":\"TemperatureSensor\"";
		str += ",\"addr\":\"";
		str += String(ows.getSensorAddress(i)) + "\"}";
	}
	str += "]";

	str += ",\"actuators\":[";
	str += "{\"name\":\"HeaterRele\"";
	str += ",\"type\":\"HeaterActuator\"";
	str += ",\"addr\":\"" + HeaterActuatorSubaddress + "\"}";
	str += "]";

	str += "}";

	str.toCharArray(buffer, buffersize);

	HttpHelper hplr;

	String result;
	boolean res = hplr.post(settings.servername, settings.serverPort, "/webduino/shield", buffer, str.length(), &result);
	logger.print(tag, "\n\tanswer = ");
	logger.println(tag, result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	logger.print(tag, "\tresult = ");
	logger.println(tag, resultvalue);

	if (resultvalue.equalsIgnoreCase("success")) {
		
		int id = json.jsonGetInt("id");

		serverTime = json.jsonGetLong("timesec");
		setSyncProvider(getNtpTime);
		digitalClockDisplay();

		return id;
	}
	else {
		return 0;
	}
}


void Command::digitalClockDisplay(){
	// digital clock display of the time
	Serial.print(hour());
	printDigits(minute());
	printDigits(second());
	Serial.print(" ");
	Serial.print(day());
	Serial.print(" ");
	Serial.print(month());
	Serial.print(" ");
	Serial.print(year());
	Serial.println();
}

void Command::printDigits(int digits){
	// utility function for digital clock display: prints preceding colon and leading 0
	Serial.print(":");
	if (digits < 10)
		Serial.print('0');
	Serial.print(digits);
}


boolean Command::sendActuatorStatus(Settings settings, OneWireSensors ows, Program programSettings)
{
	logger.println(tag, F("SEND ACTUATOR STATUS"));

	if (settings.id == 0) {
		logger.println(tag, F("ID NON VALIDO"));
		return false;
	}

	time_t remaining = programSettings.programDuration - (millis() - programSettings.programStartTime);

	const int buffersize = 500;
	char   buffer[buffersize];

	//Serial.println(statusStr[currentStatus]);
	//Serial.println(boolStr[releStatus]);

	String str = "{";
	str += "\"command\":\"status\",";
	str += "\"id\":" + String(settings.id) + ",";
	str += "\"addr\":\"" + HeaterActuatorSubaddress + "\",";
	str += "\"temperature\":";
	char temp[10];
	sprintf(temp, "%d.%02d", (int)settings.localTemperature, (int)(settings.localTemperature * 100.0) % 100);
	str += String(temp);
	str += ",";
	str += "\"avtemperature\":";
	temp[10];
	sprintf(temp, "%d.%02d", (int)settings.localAvTemperature, (int)(settings.localAvTemperature * 100.0) % 100);
	str += String(temp);
	str += ",";
	str += "\"status\":\"" + String(statusStr[programSettings.currentStatus]) + "\",";
	str += "\"type\":\"heater\",";
	str += "\"relestatus\":\"" + String((programSettings.releStatus) ? boolStr[1] : boolStr[0]) + "\",";
	str += "\"remaining\":" + String(remaining) + "";
	str += "}";

	str.toCharArray(buffer, buffersize);

	String result;
	HttpHelper hplr;
	hplr.post(settings.servername, settings.serverPort, "/webduino/actuator", buffer, str.length(), &result);
	logger.print(tag, "\n\tanswer = ");
	logger.println(tag, result);

	return true;
}

boolean Command::sendSensorsStatus(Settings settings, OneWireSensors ows)
{
	logger.println(tag, F("SEND SENSOR STATUS"));

	if (settings.id == 0) {
		logger.println(tag, F("ID NON VALIDO"));
		return false;
	}		
	
	const int buffersize = 500;
	char   buffer[buffersize];
	String str = "{";
	str += "\"id\":" + String(settings.id);// shieldid
	str += ",\"temperature\":";
	char temp[10];
	sprintf(temp, "%d.%02d", (int)settings.localTemperature, (int)(settings.localTemperature * 100.0) % 100);
	str += String(temp);
	str += ",\"avtemperature\":";
	temp[10];
	sprintf(temp, "%d.%02d", (int)settings.localAvTemperature, (int)(settings.localAvTemperature * 100.0) % 100);
	str += String(temp);
	str += ",\"sensors\":[";
	for (int i = 0; i < ows.sensorCount; i++) {

		if (i != 0)
			str += ",";
		str += "{";
		str += "\"temperature\":";
		str += Util::floatToString(ows.sensorTemperatures[i]);
		str += ",\"avtemperature\":";
		str += Util::floatToString(ows.sensorAvTemperatures[i]);
		str += ",\"name\":\"";
		str += String(ows.sensorname[i]) + "\"";
		str += ",\"type\":\"temperature\"";
		str += ",\"addr\":\"";
		str += String(ows.getSensorAddress(i)) + "\"}";
	}
	str += "]";
	str += ",\"MAC\":\"" + String(settings.MAC_char) + "\"";
	str += ",\"name\":\"" + String(settings.boardname) + "\"";
	str += "}";
	str.toCharArray(buffer, buffersize);

	String result;
	HttpHelper hplr;
	bool res = hplr.post(settings.servername, settings.serverPort, "/webduino/sensor", buffer, str.length(), &result);
	logger.print(tag, "\n\tanswer = ");
	logger.println(tag, result);
	return res;
}
