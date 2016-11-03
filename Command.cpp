#include "Command.h"
#include "Program.h"
#include "Util.h"

extern const char* statusStr[];
char* boolStr[] = { "false", "true" };

Command::Command()
{
}


Command::~Command()
{
}




boolean Command::registerShield(Settings settings, OneWireSensors ows)
{
	Serial.println(F("REGISTER SHIELD"));

	const int buffersize = 500;
	char   buffer[buffersize];
	String str = "{";

	str += "\"MAC\":\"" + String(settings.MAC_char) + "\"";
	str += ",\"boardname\":\"" + String(settings.boardname) + "\"";
	str += ",\"sensors\":[";
	for (int i = 0; i < ows.sensorCount; i++) {

		if (i != 0)
			str += ",";
		str += "{\"name\":\"";
		str += String(ows.sensorname[i]) + "\"";
		str += ",\"addr\":\"";
		//str += String(sensorAddressToString(ows.sensorAddr[i])) + "\"}";
		str += String(ows.getSensorAddress(i)) + "\"}";
	}
	str += "]";
	str += "}";

	str.toCharArray(buffer, buffersize);

	HttpHelper hplr;

	String result;
	boolean shieldRegistered = hplr.post(settings.servername, settings.serverPort/*8080*/, "/webduino/shield", buffer, str.length(), &result);
	Serial.print("answer = ");
	Serial.println(result);

	JSON* json = new JSON(result);

	String succes = json->jsonGetString("result");
	Serial.print("success = ");
	Serial.println(succes);

	String id = json->jsonGetInt("id");
	Serial.print("id = ");
	Serial.println(id);


	if (shieldRegistered) {

		return true;
	}
	else  {
		return false;
	}
}

boolean Command::sendActuatorStatus(Settings settings, OneWireSensors ows, Program programSettings)
{
	Serial.println(F("SEND ACTUATOR STATUS"));

	//if (!netwokStarted) return false;

	time_t remaining = programSettings.programDuration - (millis() - programSettings.programStartTime);

	const int buffersize = 500;
	char   buffer[buffersize];

	//Serial.println(statusStr[currentStatus]);
	//Serial.println(boolStr[releStatus]);

	String str = "{";
	str += "\"command\":\"status\",";
	str += "\"id\":" + String(settings.id) + ",";
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
	str += "\"relestatus\":\"" + String((programSettings.releStatus) ? boolStr[1] : boolStr[0]) + "\",";
	str += "\"remaining\":" + String(remaining) + "";
	str += "}";

	str.toCharArray(buffer, buffersize);

	String result;
	HttpHelper hplr;
	hplr.post(settings.servername, settings.serverPort, "/webduino/actuator", buffer, str.length(), &result);
	Serial.print("answer = ");
	Serial.print(result);

	return true;
}



boolean Command::sendSensorsStatus(Settings settings, OneWireSensors ows)
{
	/*if (!shieldRegistered) {
		Serial.println(F("shield NOT registered"));
	}*/

	Serial.println(F("SEND SENSOR STATUS"));
	const int buffersize = 500;
	char   buffer[buffersize];
	String str = "{";
	str += "\"id\":" + String(settings.id);
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
		str += "\"n\":";
		int n = i + 1;
		str += String(n);
		str += ",\"temperature\":";
		str += Util::floatToString(ows.sensorTemperatures[i]);
		str += ",\"avtemperature\":";
		str += Util::floatToString(ows.sensorAvTemperatures[i]);
		str += ",\"name\":\"";
		str += String(ows.sensorname[i]) + "\"";
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
	bool res = hplr.post(settings.servername, settings.serverPort/*8080*/, "/webduino/sensor", buffer, str.length(), &result);
	Serial.print("answer = ");
	Serial.print(result);
	return res;
}