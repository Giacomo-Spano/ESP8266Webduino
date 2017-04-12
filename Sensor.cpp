#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";


Sensor::Sensor()
{
}

Sensor::~Sensor()
{
}

String Sensor::getJSON() {

	logger.print(tag, "\n\t>>getJSON");

	String json = "";
	json += "{";

	// common field
	json += "\"type\":\""+ type + "\"";
	json += ",\"name\":\"";
	json += String(sensorname) + "\"";
	json += ",\"enabled\":";
	if (enabled == true)
		json += "true";
	else
		json += "false";
	json += ",\"pin\":\"" + Shield::getStrPin(pin) + "\"";
	json += ",\"addr\":\"";
	json += String(getSensorAddress()) + "\"";
	
	// get custom json field
	json += getJSONFields();
	
	json += "}";

	logger.print(tag, "\n\t<<getJSON json=" + json);
	return json;
}

String Sensor::getJSONFields()
{
	return String();
}

void Sensor::init()
{
}

String Sensor::getSensorAddress() {

	/*String str = "";
	for (int i = 0; i < 8; i++) {

		char temp[30];
		sprintf(temp, "%02X", sensorAddr[i]);
		str += String(temp);

		if (i < 7) {
			str += ":";
		}
	}
	return str;*/
	return address;
}


