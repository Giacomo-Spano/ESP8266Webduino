#include "Sensor.h"



Sensor::Sensor()
{
}


Sensor::~Sensor()
{
}

String Sensor::getJSON() {
	return "Sensor";
}

String Sensor::getSensorAddress() {

	String str = "";
	for (int i = 0; i < 8; i++) {

		char temp[30];
		sprintf(temp, "%02X", sensorAddr[i]);
		str += String(temp);

		if (i < 7) {
			str += ":";
		}
	}
	return str;
}
