#include "OneWireSensors.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D4
extern uint8_t provapin;
OneWire _oneWire(provapin);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature _sensors(&_oneWire);

OneWireSensors::OneWireSensors()
{
	
}


OneWireSensors::~OneWireSensors()
{
}

void OneWireSensors::discoverOneWireDevices(void) {

	_sensors.begin();

	Serial.println("discoverOneWireDevices...\n\r");

	byte i;

	Serial.print("Looking for 1-Wire devices...\n\r");
	while (sensorCount < maxSensorNumber && _oneWire.search(sensorAddr[sensorCount])) {
		Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
		for (i = 0; i < 8; i++) {
			Serial.print("0x");
			if (sensorAddr[sensorCount][i] < 16) {
				Serial.print('0');
			}
			Serial.print(sensorAddr[sensorCount][i], HEX);
			if (i < 7) {
				Serial.print(", ");
			}
		}
		if (OneWire::crc8(sensorAddr[sensorCount], 7) != sensorAddr[sensorCount][7]) {
			Serial.print("CRC is not valid!\n");
			return;
		}
		sensorCount++;
	}
	Serial.print("\n\r\n\rThat's it.\r\n");
	_oneWire.reset_search();
	return;
}

float OneWireSensors::readTemperatures(){

	// call sensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
	Serial.print("Requesting temperatures...");
	_sensors.requestTemperatures(); // Send the command to get temperatures
	Serial.println("DONE");
	// After we got the temperatures, we can print them here.
	// We use the function ByIndex, and as an example get the temperature from the first sensor only.

	for (int i = 0; i < sensorCount; i++) {

		Serial.print("Temperature for the device ");
		Serial.print(sensorname[i]);
		Serial.print(" is: ");
		float dallasTemperature = _sensors.getTempC(sensorAddr[i]);
		Serial.println(dallasTemperature);

		sensorTemperatures[i] = dallasTemperature;
		avTemp[i][avTempCounter] = dallasTemperature;

		sensorAvTemperatures[i] = getAverageTemperature(i);

	}

	avTempCounter++;
	if (avTempCounter >= avTempsize)
		avTempCounter = 0;

	return sensorTemperatures[0];
}

float OneWireSensors::getAverageTemperature(int n) {

	float average = 0;

	for (int i = 0; i < avTempsize; i++) {
		average += avTemp[n][i];
	}
	average = average / (avTempsize);

	return average;
}

String OneWireSensors::getSensorAddress(int n) {

	String str = "";
	for (int i = 0; i < 8; i++) {

		char temp[30];
		sprintf(temp, "%02X", sensorAddr[n][i]);
		str += String(temp);

		if (i < 7) {
			str += ":";
		}
	}
	return str;
}

