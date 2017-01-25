#include "DS18S20Sensor.h"
#include "Logger.h"
#include "Util.h"
#include "ESP8266Webduino.h"


extern Logger logger;
extern uint8_t OneWirePin;
////extern OneWire oneWire;
//extern OneWire *oneWirePtr;
//extern DallasTemperature dallasSensors;
//extern DallasTemperature* pDallasSensors;


DS18S20Sensor::DS18S20Sensor()
{
	tag = "DS18S20Sensor";	
}

DS18S20Sensor::~DS18S20Sensor()
{
}

float DS18S20Sensor::getTemperature() {
	return temperature;
}

float DS18S20Sensor::getAvTemperature() {
	return avTemperature;
}

void DS18S20Sensor::readTemperature(DallasTemperature *pDallasSensors){

	// call dallasSensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
	logger.print(tag, "\n\tsensor: ");
	logger.print(tag, sensorname);
	logger.print(tag, "\n\taddr ");
	logger.print(tag, getSensorAddress());
	pDallasSensors->requestTemperatures(); // Send the command to get temperatures
	
	logger.print(tag, "\n\told Temperature   is: ");
	logger.print(tag, String(temperature));

	float dallasTemperature = pDallasSensors->getTempC(sensorAddr);
	logger.print(tag, "\n\tdallas Temperature   is: ");
	logger.print(tag, String(dallasTemperature));
	
	temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
	logger.print(tag, "\n\trounded Temperature  is: ");
	logger.print(tag, String(temperature));
			
	if (avTempCounter < avTempsize) {
		avTemp[avTempCounter] = temperature;
		avTempCounter++;
	} else {
		for (int i = 0; i < avTempCounter - 1; i++)
		{
			avTemp[i] = avTemp[i + 1];
		}
		avTemp[avTempCounter-1] = temperature;
	}
	float average = 0.0;
	for (int i = 0; i < avTempCounter; i++) {
		average += avTemp[i];
	}
	average = average / (avTempCounter);
	avTemperature = ((int)(average * 100 + .5) / 100.0);
	
	logger.print(tag, "\n\tAverage temperature  is: ");
	logger.print(tag, String(avTemperature));
	logger.print(tag, "\n");
}

String DS18S20Sensor::getJSON() {
	String json = "";
	json += "{";
	json += "\"temperature\":";
	//json += Util::floatToString(temperature);
	json += String(getTemperature());
	json += ",\"avtemperature\":";
	//json += Util::floatToString(avTemperature);
	json += String(getAvTemperature());
	json += ",\"name\":\"";
	json += String(sensorname) + "\"";
	json += ",\"type\":\"temperature\"";
	json += ",\"addr\":\"";
	json += String(getSensorAddress()) + "\"}";
	return json;
}

