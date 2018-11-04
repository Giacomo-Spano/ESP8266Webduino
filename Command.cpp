#include "Command.h"
#include "Util.h"
#include "MQTTMessage.h"

extern bool mqtt_publish(MQTTMessage mqttmessage);

Logger Command::logger;
String Command::tag = "Command";

Command::Command()
{
}

Command::~Command()
{
}

bool Command::requestShieldSettings(String MACAddress, String rebootreason)
{
	logger.print(tag, F("\n\t>>Command::requestShieldSettings "));
	logger.print(tag, MACAddress);

	MQTTMessage mqttmessage;
	bool res = false;
	mqttmessage.topic = "toServer/shield/loadsettings";

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["rebootreason"] = rebootreason;
	json["MACAddress"] = MACAddress;
	logger.printJson(json);

	json.printTo(mqttmessage.message);
	res = mqtt_publish(mqttmessage);
	if (!res)
		logger.print(tag, F("\n\t<<Command::requestShieldSettings failed"));
	logger.print(tag, F("\n\t<<Command::requestShieldSettings"));
	return res;
}

bool Command::requestTime(String macAddress)
{
	logger.print(tag, F("\n\t >>requestTime\n"));
	MQTTMessage mqttmessage;
	mqttmessage.topic = "toServer/shield/time";
	mqttmessage.message = macAddress;
	bool res = mqtt_publish(mqttmessage);
	logger.print(tag, F("\n\t <<requestTime res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

boolean Command::sendSensorStatus(JsonObject& json)
{
	logger.print(tag, F("\n\t >>Command::sendSensorStatus"));

	bool res = false;
	MQTTMessage mqttmessage;
	mqttmessage.topic = "toServer/shield/sensor/update";
	//mqttmessage.jsonStr;
	//logger.print(tag, F("\n\t jsonStr="));
	logger.printJson(json);
	json.printTo(mqttmessage.message);
	res = mqtt_publish(mqttmessage);
	logger.print(tag, F("\n\t <<Command::sendSensorStatus res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

boolean Command::sendStrSensorStatus(String strJson)
{
	logger.print(tag, F("\n\t >>Command::sendStrSensorStatus"));

	bool res = false;
	MQTTMessage mqttmessage;
	mqttmessage.topic = "toServer/shield/sensor/update";
	//logger.printJson(json);
	//json.printTo(mqttmessage.message);
	mqttmessage.message = strJson;
	res = mqtt_publish(mqttmessage);
	logger.print(tag, Logger::boolToString(res));
	logger.print(tag, F("\n\t <<Command::sendStrSensorStatus res="));
	
	return res;
}


boolean Command::sendShieldStatus(String json)
{
	logger.print(tag, F("\n\t>>sendShieldStatus"));
	bool res = false;
	MQTTMessage mqttmessage;
	mqttmessage.topic = "toServer/shield/update";
	mqttmessage.message = json;
	res = mqtt_publish(mqttmessage);
	logger.print(tag, F("\n\t<<sendShieldStatus\n res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

boolean Command::requestZoneTemperature(int id, String json)
{
	logger.print(tag, F("\n"));
	logger.println(tag, F(">>Command::requestZoneTemperature"));

	if (id == 0) {
		logger.print(tag, F("\n\tID NON VALIDO"));
		return false;
	}
	MQTTMessage mqttmessage;
	bool res = false;
	mqttmessage.topic = "toServer/shield/" + String(id) + String("/requestzonetemperature");
	mqttmessage.message = json;
	res = mqtt_publish(mqttmessage);

	logger.println(tag, F("<<Command::requestZoneTemperature\n"));
	return res;
}



