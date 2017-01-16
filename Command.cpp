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
//String Command::HeaterActuatorSubaddress = "01";


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
	//str += "\"id\":\"" + /*String(shield.id) + */"\"" + "}";

	HttpHelper hplr;
	String result;
	boolean res = hplr.post(/*shield.servername*/"http://192.168.1.3",8080/*shield.serverPort*/, "/webduino/time", str, &result);
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

bool Command::sendLog(String log/*, /*int shieldid, String servername, int port*/)
{
	//Serial.println(F("SENDLOG"));
	if (log.length() > Command::maxLogSize)
		return false;

	String packetend = "#END#";
	String packetstart = "#START#";
	String packetnumber = String(packetcount++);
	String strid = String(Shield::getShieldId());
	if (packetcount > 99999)
		packetcount = 0;

	String json = packetstart + ":" + packetnumber + ":" + strid + ":" + log + ":" + packetend;

	HttpHelper hplr;
	String result;
	boolean res = hplr.post(Shield::servername, Shield::serverPort, "/webduino/log", json, &result);

	JSON jsonResult(result);
	String resultvalue = jsonResult.jsonGetString("result");

	if (resultvalue.equalsIgnoreCase("success")) {

		return true;
	}
	else {
		return false;
	}
}

int Command::registerShield(Shield shield)
{
	logger.println(tag, F("REGISTER SHIELD"));


	String str = "{";
	str += "\"event\":\"register\",";
	str += "\"shield\": ";
	

	str += "{";
	str += "\"MAC\":\"" + String(shield.MAC_char) + "\"";
	str += ",\"boardname\":\"" + String(shield.boardname) + "\"";
	str += ",\"localIP\":\"" + shield.localIP + "\"";
	str += ",\"localPort\":\"" + String(shield.localPort) + "\"";
	
	// sensori
	str += ",\"sensors\":[";
	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		if (i != 0)
			str += ",";
		str += sensor->getJSON();
	}
	str += "]";

	// attuatori
	str += ",\"actuators\":[";
	str += shield.hearterActuator.getJSON();
	str += "]";
	str += "}";
	
	str += "}";

	HttpHelper hplr;

	String result;
	boolean res = hplr.post(shield.servername, shield.serverPort, "/webduino/shield", str, &result);
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

int Command::timeSync(/*String servername, int port*/)
{
	logger.println(tag, F("timeSync"));

	String str = "{";
	str += "}";

	HttpHelper hplr;

	Shield shield;
	String result;
	boolean res = hplr.post(shield.getServerName(), shield.getServerPort(), "/webduino/time", str, &result);
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

boolean Command::sendActuatorStatus(HeaterActuator actuator)
{
	logger.println(tag, F("SEND ACTUATOR STATUS"));

	//Shield shield;

	if (Shield::getShieldId() == 0) {
		logger.println(tag, F("ID NON VALIDO"));
		return false;
	}

	String jsonActuator = actuator.getJSON();

	String str = "{";
	str += "\"event\":\"update\",";
	str += "\"actuator\": " + jsonActuator + "}";
	
	String result;
	HttpHelper hplr;
	
	hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/actuator", str, &result);
	logger.println(tag, "\n\tanswer = ");
	logger.println(tag, result);

	return true;
}

boolean Command::sendSensorsStatus(Shield shield)
{
	logger.println(tag, F("SEND SENSOR STATUS"));

	if (Shield::getShieldId() == 0) {
		logger.print(tag, F("\n\tID NON VALIDO"));
		return false;
	}

	String str = shield.getSensorsStatusJson();
	
	logger.print(tag, F("\n\tjson="));
	logger.print(tag, str);

	String result;
	HttpHelper hplr;
	bool res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/sensor", str, &result);
	logger.print(tag, "\n\tanswer = ");
	logger.print(tag, result);
	return res;
}

boolean Command::download(String filename, Shield shield)
{
	logger.println(tag, F("download"));

	
	String str = "{";
	str += "\"filenamae\":\"status\",";
	
	str += "}";

	
	String result;
	HttpHelper hplr;
	hplr.downloadfile(filename, shield.servername, shield.serverPort, "/webduino/"+filename, str, &result);
	//logger.print(tag, "\n\tanswer = ");
	//logger.println(tag, result);
	//Serial.print(result);

	return true;
}
