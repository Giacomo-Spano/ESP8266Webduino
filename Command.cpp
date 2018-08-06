#include "Command.h"
#include "Program.h"
#include "Util.h"
#include "JSONObject.h"

extern bool mqtt_publish(String topic, String message);
//extern bool _mqtt_publish(char* topic, char* payload);

Logger Command::logger;
String Command::tag = "Command";

Command::Command()
{
}

Command::~Command()
{
}

bool Command::requestShieldSettings()
{
	logger.print(tag, F("\n\t>>Command::requestShieldSettings"));

	bool res = false;
	if (Shield::getMQTTmode() == true) {
		String topic = "toServer/shield/loadsettings";// +Shield::getMACAddress();
		String json = Shield::getMACAddress();
		res = mqtt_publish(topic, String(json));
	}
	if (!res)
		logger.print(tag, F("\n\t<<Command::requestShieldSettings failed"));
	logger.print(tag, F("\n\t<<Command::requestShieldSettings"));
	return res;
}

bool Command::requestTime()
{
	logger.print(tag, F("\n\t>> requestTime\n"));

	String topic = "toServer/shield/time";
	String json = Shield::getMACAddress();
	bool res = mqtt_publish(topic, String(json));
	logger.print(tag, F("\n\t<< requestTime\n"));
	return res;
}

boolean Command::sendSensorStatus(String json)
{
	logger.print(tag, F("\n\t >>Command::sendSensorStatus"));
		
	bool res = false;
	String topic = "toServer/shield/sensor/update";
	res = mqtt_publish(topic, String(json));
	logger.print(tag, "\n\t <<Command::sendSensorStatus res=" + Logger::boolToString(res));
	return res;
}

boolean Command::sendShieldStatus(String json)
{
	logger.print(tag, F("\n\t>>sendShieldStatus"));
	bool res = false;
	if (Shield::getMQTTmode() == true) {
		String topic = "toServer/shield/update";
		res = mqtt_publish(topic, String(json));
	}
	logger.print(tag, "\n\t<<sendShieldStatus\n res=" + Logger::boolToString(res));
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



