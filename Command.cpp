#include "Command.h"
#include "Program.h"
#include "Util.h"
#include "DS18S20Sensor.h"

extern Logger logger;

//extern time_t getNtpTime();
extern const char* statusStr[];
//char* boolStr[] = { "false", "true" };

int packetcount = 1;

Command* Command::getTimeObject;
String Command::HeaterActuatorSubaddress = "01";


void digitalClockDisplay(){
	// digital clock display of the time
	/*Serial.print(hour());
	printDigits(minute());
	printDigits(second());
	Serial.print(" ");
	Serial.print(day());
	Serial.print(" ");
	Serial.print(month());
	Serial.print(" ");
	Serial.print(year());
	Serial.println();*/
}
time_t serverTime =11111111;

time_t Command::getNtpTime() {
	return serverTime;
}


time_t xgetNtpTime() {


	Serial.println("BBB");

	String tag = "getNtpTime";

	//logger.println(tag, F("getNtpTime"));
#ifdef dopo
	String str = "{";
	//str += "\"id\":\"" + /*String(settings.id) + */"\"" + "}";

	HttpHelper hplr;
	String result;
	boolean res = hplr.post(/*settings.servername*/"http://192.168.1.3",8080/*settings.serverPort*/, "/webduino/time", str, &result);
	Serial.println("CCC");

	//logger.print(tag, "\n\tanswer = ");
	//logger.println(tag, result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	//logger.print(tag, "\tresult = ");
	//logger.println(tag, resultvalue);

	if (resultvalue.equalsIgnoreCase("success")) {

		serverTime = json.jsonGetLong("timesec");
		Serial.println("serverTime=");
		Serial.println(serverTime);
		//setSyncProvider(globalGetNTPTime/*getNtpTime*/);
	}
#endif
	return serverTime;
}

void printDigits(int digits){
	// utility function for digital clock display: prints preceding colon and leading 0
	Serial.print(":");
	if (digits < 10)
		Serial.print('0');
	Serial.print(digits);
}
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

	String json = packetstart + ":" + packetnumber + ":" + strid + ":" + log + ":" + packetend;

	HttpHelper hplr;
	String result;
	boolean res = hplr.post(servername, port, "/webduino/log", json, &result);

	JSON jsonResult(result);
	String resultvalue = jsonResult.jsonGetString("result");

	if (resultvalue.equalsIgnoreCase("success")) {

		return true;
	}
	else {
		return false;
	}
}

int Command::registerShield(Settings settings)
{
	logger.println(tag, F("REGISTER SHIELD"));

	String str = "{";
	str += "\"MAC\":\"" + String(settings.MAC_char) + "\"";
	str += ",\"boardname\":\"" + String(settings.boardname) + "\"";
	str += ",\"localIP\":\"" + settings.localIP + "\"";
	str += ",\"localPort\":\"" + String(settings.localPort) + "\"";
	str += ",\"sensors\":[";

	DS18S20Sensor* elem = (DS18S20Sensor*) settings.sensorList.getFirst();
	int count = 0;
	while (elem != nullptr) {

		logger.print(tag, "\n\telem->sensorname=" + elem->sensorname);
		
		if (count++ != 0)
				str += ",";
		str += "{\"name\":\"";
		str += String(elem->sensorname) + "\"";
		str += ",\"type\":\"TemperatureSensor\"";
		str += ",\"addr\":\"";
		str += String(elem->getSensorAddress()) + "\"}";
	
		elem = (DS18S20Sensor*)settings.sensorList.getNext();
	}

	logger.print(tag, "\n\t");

	str += "]";

	str += ",\"actuators\":[";
	str += "{\"name\":\"HeaterRele\"";
	str += ",\"type\":\"HeaterActuator\"";
	str += ",\"addr\":\"" + HeaterActuatorSubaddress + "\"}";
	str += "]";

	str += "}";

	HttpHelper hplr;

	String result;
	boolean res = hplr.post(settings.servername, settings.serverPort, "/webduino/shield", str, &result);
	//logger.print(tag, "\n\tanswer = ");
	//logger.println(tag, result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	logger.print(tag, "\tresult = ");
	logger.print(tag, resultvalue);

	if (resultvalue.equalsIgnoreCase("success")) {

		int id = json.jsonGetInt("id");
		logger.print(tag, "\n\tid = ");
		logger.print(tag, String(id));
		
		serverTime = json.jsonGetLong("timesec");
		getTimeObject = this;
		//setSyncInterval(60);
		setSyncProvider(getNtpTime);
		
		logger.print(tag, "\n\tid = ");
		logger.print(tag, String(id));
		return id;
	}
	else {
		return 0;
	}
}

int Command::timeSync(String servername, int port)
{
	logger.println(tag, F("timeSync"));

	String str = "{";
	str += "}";

	HttpHelper hplr;

	String result;
	boolean res = hplr.post(servername, port, "/webduino/time", str, &result);
	logger.print(tag, "\n\tanswer = ");
	logger.print(tag, result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	logger.print(tag, "\tresult = ");
	logger.print(tag, resultvalue);

	if (resultvalue.equalsIgnoreCase("success")) {
		serverTime = json.jsonGetLong("timesec");
		return serverTime;
	}
	else {
		return 0;
	}
}

boolean Command::sendActuatorStatus(Settings settings, HeaterActuator actuator)
{
	logger.println(tag, F("SEND ACTUATOR STATUS"));

	if (settings.id == 0) {
		logger.println(tag, F("ID NON VALIDO"));
		return false;
	}

	//time_t remaining = actuator.programDuration - (millis() - actuator.programStartTime);
	time_t remaining = actuator.getRemaininTime();

	String str = settings.getActuatorsStatusJson();
	/*String str = "{";
	str += "\"command\":\"status\",";
	str += "\"id\":" + String(settings.id) + ",";
	str += "\"addr\":\"" + HeaterActuatorSubaddress + "\",";
	str += "\"status\":\"" + String(statusStr[actuator.getStatus()]) + "\",";
	str += "\"type\":\"heater\",";
	str += "\"relestatus\":\"" + String((actuator.getReleStatus()) ? boolStr[1] : boolStr[0]) + "\",";
	str += "\"remaining\":" + String(remaining) + "";
	str += "}";*/

	
	String result;
	HttpHelper hplr;
	hplr.post(settings.servername, settings.serverPort, "/webduino/actuator", str, &result);
	logger.print(tag, "\n\tanswer = ");
	logger.println(tag, result);

	return true;
}

boolean Command::sendSensorsStatus(Settings settings)
{
	logger.println(tag, F("SEND SENSOR STATUS"));

	if (settings.id == 0) {
		logger.print(tag, F("\n\tID NON VALIDO"));
		return false;
	}

	String str = settings.getSensorsStatusJson();
	/*String str = "{";
	str += "\"id\":" + String(settings.id);// shieldid
	str += ",\"sensors\":[";
	for (int i = 0; i < settings.sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)settings.sensorList.get(i);
		if (i != 0)
			str += ",";
		str += sensor->getJSON();
	}
	str += "]";
	str += "}";*/

	logger.print(tag, F("\n\tjson="));
	logger.print(tag, str);

	String result;
	HttpHelper hplr;
	bool res = hplr.post(settings.servername, settings.serverPort, "/webduino/sensor", str, &result);
	logger.print(tag, "\n\tanswer = ");
	logger.print(tag, result);
	return res;
}

boolean Command::download(String filename, Settings settings)
{
	logger.println(tag, F("download"));

	
	String str = "{";
	str += "\"filenamae\":\"status\",";
	
	str += "}";

	
	String result;
	HttpHelper hplr;
	hplr.downloadfile(filename, settings.servername, settings.serverPort, "/webduino/"+filename, str, &result);
	//logger.print(tag, "\n\tanswer = ");
	//logger.println(tag, result);
	//Serial.print(result);

	return true;
}
