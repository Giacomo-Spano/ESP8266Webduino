// 
// 
// 

#include "OnewireSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"
#include "JSONArray.h"

Logger OnewireSensor::logger;
String OnewireSensor::tag = "OnewireSensor";

OnewireSensor::OnewireSensor(uint8_t pin, bool enabled, String address, String name) : Sensor(pin, enabled, address, name)
{
	checkStatus_interval = 60000;
	lastCheckStatus = 0;

	type = "onewiresensor";
	// beginTemperatureSensors deve essere chiamata nel costruttore per inizializzare
	// la var tempSensorNum. 
	beginTemperatureSensors();


}

/*OnewireSensor::OnewireSensor()
{
	type = "onewiresensor";
}*/

OnewireSensor::~OnewireSensor()
{
}

void OnewireSensor::init()
{
}

void OnewireSensor::beginTemperatureSensors()
{
	// questa funz può essere chiamata solo dopo
	// aver inizializzato pin
	logger.print(tag, "\n\t >>init OnewireSensor");
	oneWirePtr = new OneWire(pin);
	pDallasSensors = new DallasTemperature(oneWirePtr);
	pDallasSensors->begin();

	uint8_t address[8];
	tempSensorNum = 0;
	while (oneWirePtr->search(address) && tempSensorNum < OnewireSensor::maxTempSensors) {
		logger.print(tag, "\n\tFound \'1-Wire\' device with address:");
		for (int i = 0; i < 8; i++) {
			logger.print(tag, "0x");
			if (address[i] < 16) {
				logger.print(tag, '0');
			}
			logger.print(tag, address[i]);
			if (i < 7) {
				logger.print(tag, ", ");
			}
		}
		if (OneWire::crc8(address, 7) != address[7]) {
			logger.print(tag, "\n\tCRC is not valid!");
			return;
		}

		for (int i = 0; i < 8; i++) {
			temperatureSensors[tempSensorNum].sensorAddr[i] = address[i];
		}
		logger.print(tag, "\n\t tempSensorNum.name = " + String(tempSensorNum));

		temperatureSensors[tempSensorNum].id = tempSensorNum + 1;
		temperatureSensors[tempSensorNum].name = "sensore temperatura" + String(tempSensorNum + 1);
		
		logger.print(tag, "\n\t temperatureSensors[tempSensorNum].name = " + temperatureSensors[tempSensorNum].name);
		logger.print(tag, "\n\t temperatureSensors[tempSensorNum].id = " + String(temperatureSensors[tempSensorNum].id));

		tempSensorNum++;

	}
	oneWirePtr->reset_search();

	logger.print(tag, "\n\t <<init OnewireSensor");
}

/*uint8_t OnewireSensor::search(uint8_t *address) {
	return oneWirePtr->search(address);
}

void OnewireSensor::reset_search() {
	return oneWirePtr->reset_search();
}*/


float OnewireSensor::getTemperature(int index) {
	if (index < 0 || index > maxTempSensors)
		return -99;

	return temperatureSensors[index].temperature;
}

float OnewireSensor::getAvTemperature(int index) {
	if (index < 0 || index > maxTempSensors)
		return -99;

	return temperatureSensors[index].avTemperature;
}

bool OnewireSensor::readTemperatures() {

	logger.print(tag, "\n\n\t >>readTemperatures");
	// questa funzione ritorna true se è cambiata almeno uan tempertura
	int res = false; // 

	pDallasSensors->requestTemperatures(); // Send the command to get temperatures

	logger.print(tag, "\n\t tempSensorNum: " + String(tempSensorNum));
	for (int i = 0; i < tempSensorNum; i++) {

		// call dallasSensors.requestTemperatures() to issue a global temperature 
		// request to all devices on the bus
		logger.print(tag, "\n\t sensor: ");
		logger.print(tag, temperatureSensors[i].name);
		logger.print(tag, "\n\t index: ");
		logger.print(tag, i);
		logger.print(tag, "\n\t addr ");
		logger.print(tag, temperatureSensors[i].getPhisicalAddress());

		float oldTemperature = temperatureSensors[i].temperature;
		logger.print(tag, "\n\t old Temperature   is: ");
		logger.print(tag, String(temperatureSensors[i].temperature));

		float dallasTemperature = pDallasSensors->getTempC(temperatureSensors[i].sensorAddr);
		logger.print(tag, "\n\t dallas Temperature   is: ");
		logger.print(tag, String(dallasTemperature));

		temperatureSensors[i].temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		logger.print(tag, "\n\t rounded Temperature  is: ");
		logger.print(tag, String(temperatureSensors[i].temperature));

		// se è cambiata almeno una temperatura ritorna true
		if (oldTemperature != temperatureSensors[i].temperature)
			res = true;

		if (avTempCounter < avTempsize) {
			avTemp[avTempCounter] = temperatureSensors[i].temperature;
			avTempCounter++;
		}
		else {
			for (int i = 0; i < avTempCounter - 1; i++)
			{
				avTemp[i] = avTemp[i + 1];
			}
			avTemp[avTempCounter - 1] = temperatureSensors[i].temperature;
		}
		float average = 0.0;
		for (int i = 0; i < avTempCounter; i++) {
			average += avTemp[i];
		}
		average = average / (avTempCounter);
		temperatureSensors[i].avTemperature = ((int)(average * 100 + .5) / 100.0);

		logger.print(tag, "\n\tAverage temperature  is: ");
		logger.print(tag, String(temperatureSensors[i].avTemperature));
		logger.print(tag, "\n");

	}
	
	if(res)
		logger.print(tag, "\n\n\t >>readTemperatures - temperatura cambiata");
	else
		logger.print(tag, "\n\n\t >>readTemperatures - temperatura non cambiata");
	
	return res;
}

String OnewireSensor::getJSONFields() {

	logger.print(tag, "\n\t >>OnewireSensor::getJSONFields");

	String json = "";
	//json += ",\"addr\":";
	//json += "\"" + getSensorAddress() + "\"";

	json += ",\"childsensors\":[";

	for (int i = 0; i < tempSensorNum; i++) {
		if (i > 0)
			json += ",";
		json += "{";
		json += "\"id\":";
		json += String(temperatureSensors[i].id);
		json += ",\"name\":";
		json += "\"" + temperatureSensors[i].name + "\"";
		json += ",\"addr\":";
		json += "\"" + address + "." + String(temperatureSensors[i].id) + "\"";
		json += ",\"phisicaladdr\":";
		json += "\"" + temperatureSensors[i].getPhisicalAddress() + "." + String(temperatureSensors[i].id) + "\"";
		json += ",\"temperature\":";
		json += String(getTemperature(i));
		json += ",\"avtemperature\":";
		json += String(getAvTemperature(i));
		json += "}";
	}

	json += "]";

	logger.print(tag, "\n\t <<OnewireSensor::getJSONFields json=" + json);
	return json;
}

void OnewireSensor::addTemperatureSensorsFromJson(JSON sensorJson) {

	if (sensorJson.has("childsensors")) {
		String names = "";
		String str = sensorJson.jsonGetArrayString("childsensors");
		logger.print(tag, "\n\t str=" + str);
		JSONArray jArrayTempSensor(str);
		String tempSensor = jArrayTempSensor.getFirst();
		int n = 0;// tempSensorNum = 0;
		while (!tempSensor.equals("") && /*tempSensorNum*/n < OnewireSensor::maxTempSensors) {
			logger.print(tag, "\n\t tempSensor=" + tempSensor);
			tempSensor.replace("\\", "");// questo serve per correggere un baco. Per qualche motivo
										 // dalla pagina jscrit arrivano dei caratteri \ in più
			logger.print(tag, "\n\t tempSensor=" + tempSensor);
			JSON jTempSensor(tempSensor);
			if (jTempSensor.has("name")) {
				temperatureSensors[n].name = jTempSensor.jsonGetString("name");
				//temperatureSensors[n].id = n + 1;
				n++;
			}
			tempSensor = jArrayTempSensor.getNext();
		}
	}
}

bool OnewireSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	//logger.println(tag, "\n\t currMillis="+String(currMillis) + "timeDiff=" + String(timeDiff));
	if (timeDiff > checkStatus_interval) {
		logger.println(tag, F("\n\t checkTemperatures"));
		lastCheckStatus = currMillis;
		bool temperatureChanged = readTemperatures();
		logger.println(tag, "\n\t temperatureChanged" + String(temperatureChanged));
		return temperatureChanged;
	}

	return false;
}
