#include "Command.h"
#include "Program.h"
#include "Util.h"
#include "JSONObject.h"

extern bool mqtt_publish(String topic, String message);

//extern const char* statusStr[];
time_t serverTime = 11111111;
int packetcount = 1;

Logger Command::logger;
String Command::tag = "Command";

time_t Command::getNtpTime() {

	return serverTime;
}

Command::Command()
{
	//tag = "Command";
}

Command::~Command()
{
}

bool Command::sendLog(String log)
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
	boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/log", json, &result);

	JSON jsonResult(result);
	String resultvalue = jsonResult.jsonGetString("result");

	if (resultvalue.equalsIgnoreCase("success")) {
		return true;
	}
	else {
		return false;
	}
}

void Command::sendRestartNotification()
{
	logger.println(tag, F(">> sendRestartNotification\n"));

	String str = "{";
	str += "\"jsonevent\":\"restart\",";
	str += "\"reason\": \"empty\"";
	str += "}";

	HttpHelper hplr;

	String result;
	boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/shield", str, &result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	logger.print(tag, "\tresult = ");
	logger.print(tag, resultvalue);

	logger.println(tag, F("<< sendRestartNotification\n"));
}

void Command::registerShield(String json)
{
	logger.print(tag, F("\n"));
	logger.println(tag, F(">> registerShield\n"));

	if (Shield::getMQTTmode() == true) {
		String topic = "toServer/register";
		mqtt_publish(topic, String(json));
		return;
	}
	else {

		HttpHelper hplr;

		String result;
		logger.print(tag, "\n\t serverName=" + Shield::getServerName() + "**");
		logger.print(tag, "\n\t serverPort=" + String(Shield::getServerPort()));

		boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/shield", json, &result);

		JSON json(result);
		String resultvalue = json.jsonGetString("result");
		logger.print(tag, "\tresult = ");
		logger.print(tag, resultvalue);

		int id = 0;
		if (resultvalue.equalsIgnoreCase("success")) {

			id = json.jsonGetInt("id");

			serverTime = json.jsonGetLong("timesec");
			setSyncProvider(getNtpTime);
		}

		logger.print(tag, "\n\tid = ");
		logger.print(tag, String(id));

		logger.println(tag, F("<< registerShield\n"));
		Shield::setShieldId(id);
	}
	logger.println(tag, F("<<registerShield\n"));
}

bool Command::loadShieldSettings(String *result)
{
	logger.print(tag, F("\n"));
	logger.println(tag, F(">> loadShieldSettings\n"));

	String jsonevent = "{";
	jsonevent += "\"event\":\"loadsettings\",";
	jsonevent += "\"MAC\":\"" + Shield::getMACAddress() + "\"";
	//jsonevent += "\"swversion\":\"" + Shield::getSWVersion() + "\"";
	jsonevent += "}";

	logger.print(tag, "\n\t jsonevent=" + jsonevent + "\n");
		
	HttpHelper hplr;
	//String result;
	logger.print(tag, "\n\t serverName=" + Shield::getServerName() + "**");
	logger.print(tag, "\n\t serverPort=" + String(Shield::getServerPort()));
	boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/shield", jsonevent, result);

	
	logger.println(tag, "<<loadShieldSettings\n result=" + *result);

	return res;
}

int Command::timeSync()
{
	logger.print(tag, F("\n\t >>timeSync\n"));

	String str = "{";
	str += "}";

	HttpHelper hplr;

	Shield shield;
	String result;
	boolean res = hplr.post(Shield::getServerName(), Shield::getServerPort(), "/webduino/time", str, &result);
	logger.print(tag, "\n\tanswer = " + result);

	JSON json(result);
	String resultvalue = json.jsonGetString("result");
	logger.print(tag, "\tresult = ");
	logger.print(tag, resultvalue);

	time_t srvTime = 0;
	if (resultvalue.equalsIgnoreCase("success")) {
		serverTime = json.jsonGetLong("timesec");
		logger.println(tag, "\n\t <<timeSync\n" + String(serverTime));
		return serverTime;
	}

	logger.print(tag, F("\n\t <<timeSync failed\n"));
	return 0;
}


boolean Command::sendSensorsStatus(String json)
{
	logger.print(tag, "\n");
	logger.println(tag, F(">>sendSensorsStatus"));

	if (Shield::getShieldId() == 0) {
		logger.print(tag, F("\n\tID NON VALIDO"));
		return false;
	}

	//logger.println(tag, F("..calling shield.getSensorsStatusJson()\n"));
	//String json = shield.getSensorsStatusJson();
	//logger.println(tag, F("..returning from call shield.getSensorsStatusJson()\n"));

	//logger.println(tag, F("..calling logger.formattedJson(json)\n"));
	//logger.print(tag, F("\n\tjson="));
	//logger.print(tag, logger.formattedJson(json));
	//logger.println(tag, F("..returning from call logger.formattedJson(json)\n"));


	bool res = false;
	if (Shield::getMQTTmode() == true) {
		String topic = "toServer/shield/" + String(Shield::getShieldId()) + String("/sensorsupdate");
		res = mqtt_publish(topic, String(json));
	}

	logger.println(tag, F("<<sendSensorsStatus\n"));
	return res;
}

boolean Command::requestZoneTemperature(String json)
{
	logger.print(tag, "\n");
	logger.println(tag, F(">>Command::requestZoneTemperature"));

	if (Shield::getShieldId() == 0) {
		logger.print(tag, F("\n\tID NON VALIDO"));
		return false;
	}

	bool res = false;
	if (Shield::getMQTTmode() == true) {
		String topic = "toServer/shield/" + String(Shield::getShieldId()) + String("/requestzonetemperature");
		res = mqtt_publish(topic, String(json));
	}

	logger.println(tag, F("<<Command::requestZoneTemperature\n"));
	return res;
}
boolean Command::sendSettingsStatus(Shield shield)
{
	logger.println(tag, F("\n\t >>sendSettingsStatus"));

	if (Shield::getShieldId() == 0) {
		logger.print(tag, String("\n\t ID NON VALIDO ") + String(Shield::getShieldId()));
		return false;
	}

	String json = shield.getSettingsJson();

	String topic = "toServer/shield/" + String(Shield::getShieldId()) + String("/settingsupdate");
	bool res = mqtt_publish(topic, String(json));

	logger.println(tag, String("\n\t <<sendSensorsStatus\n") + String(res));
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
	hplr.downloadfile(filename, Shield::getServerName(), Shield::getServerPort(), "/webduino/" + filename, str, &result);
	//logger.print(tag, "\n\tanswer = ");
	//logger.println(tag, result);
	//Serial.print(result);

	return true;
}
