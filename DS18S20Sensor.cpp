#include "DS18S20Sensor.h"
#include "Logger.h"
#include "Util.h"


extern Logger logger;
extern uint8_t OneWirePin;
extern OneWire oneWire;
extern DallasTemperature sensors;


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

void DS18S20Sensor::readTemperature(){

	// call sensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
	logger.print(tag, "\n\tsensor: ");
	logger.print(tag, sensorname);
	logger.print(tag, "\n\taddr ");
	logger.print(tag, getSensorAddress());
	sensors.requestTemperatures(); // Send the command to get temperatures
	
	logger.print(tag, "\n\told Temperature   is: ");
	logger.print(tag, String(temperature));

	float dallasTemperature = sensors.getTempC(sensorAddr);
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



/*float DS18S20Sensor::getTemp(){
	//returns the temperature from one DS18S20 in DEG Celsius

	byte data[12];
	//byte addr[8];

	if (!oneWire.search(sensorAddr)) {
		//no more sensors on chain, reset search
		oneWire.reset_search();
		return -1000;
	}

	if (OneWire::crc8(sensorAddr, 7) != sensorAddr[7]) {
		Serial.println("CRC is not valid!");
		return -1000;
	}

	if (sensorAddr[0] != 0x10 && sensorAddr[0] != 0x28) {
		Serial.print("Device is not recognized");
		return -1000;
	}

	oneWire.reset();
	oneWire.select(sensorAddr);
	oneWire.write(0x44, 1); // start conversion, with parasite power on at the end

	byte present = oneWire.reset();
	oneWire.select(sensorAddr);
	oneWire.write(0xBE); // Read Scratchpad


	for (int i = 0; i < 9; i++) { // we need 9 bytes
		data[i] = oneWire.read();
	}

	oneWire.reset_search();

	byte MSB = data[1];
	byte LSB = data[0];

	float tempRead = ((MSB << 8) | LSB); //using two's compliment
	float TemperatureSum = tempRead / 16;

	return TemperatureSum;

}*/
