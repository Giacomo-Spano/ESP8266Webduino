// 
// 
// 

#include "OnewireSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"
#include "JSONArray.h"
#include "TemperatureSensor.h"
#include "SensorFactory.h"

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

void OnewireSensor::loadChildren(JSONArray jarray) {

	logger.print(tag, "\n\t >>loadChildren");
		
	String child = jarray.getFirst();
	int current = 0;
	while (child != "") {

		Sensor* sensor = (Sensor*)childsensors.get(current++);

		
		JSONObject json(child);
		if (json.has("name")) {
			String name = json.getString("name");
			logger.print(tag, "\n\t name=" + name);
			sensor->sensorname = name;
		}
		child = jarray.getNext();
	}

	logger.print(tag, "\n\t <<loadChildren");
}


JSONObject OnewireSensor::getJSON2() {

	logger.print(tag, "\n\t>>OnewireSensor::getJSON2");

	JSONObject jObject = Sensor::getJSON2();

	/*jObject.pushInteger("id", id);
	jObject.pushString("phisicaladdr", getPhisicalAddress());
	jObject.pushFloat("temperature", temperature);
	jObject.pushFloat("avtemperature", avTemperature);*/

	logger.print(tag, "\n\t<<OnewireSensor::getJSON2");
	return jObject;
}

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
	logger.print(tag, "\n\n\t >>beginTemperatureSensors");
	oneWirePtr = new OneWire(pin);
	pDallasSensors = new DallasTemperature(oneWirePtr);
	pDallasSensors->begin();

	uint8_t _address[8];
	tempSensorNum = 0;

	SensorFactory factory;
	childsensors.clearAll();

	while (oneWirePtr->search(_address) && tempSensorNum < OnewireSensor::maxTempSensors) {
		
		// cerca il prossimo sensore di temperatura reali attaccato allo stesso pin
		logger.print(tag, "\n\tFound \'1-Wire\' device with _address:");
		for (int i = 0; i < 8; i++) {
			logger.print(tag, "0x");
			if (_address[i] < 16) {
				logger.print(tag, '0');
			}
			logger.print(tag, _address[i]);
			if (i < 7) {
				logger.print(tag, ", ");
			}
		}
		if (OneWire::crc8(_address, 7) != _address[7]) {
			logger.print(tag, "\n\t CRC is not valid!");
			return;
		}

		// crea un nuovo TemperatreSensor assegnadogli l'address fisico e dei valori
		// di nome temporanei
		// L'eventuale nome personalizzzato
		// è caricato successivamente dalla loadChildren. Non si può fare qui!

		int id = tempSensorNum + 1;
		String name = "sensoretemperatura" + String(id);
		String subaddress = address + "." + id;
		TemperatureSensor* child = (TemperatureSensor*)factory.createSensor("temperaturesensor", pin, true, subaddress, name);
		child->id = id;
		for (int i = 0; i < 8; i++) {
			child->sensorAddr[i] = _address[i];
		}

		childsensors.add(child);
		tempSensorNum++;
	}
	oneWirePtr->reset_search();

	logger.print(tag, "\n\t <<beginTemperatureSensors\n");
}

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

	logger.print(tag, "\n\t childsensors.length(): " + String(childsensors.length()));
	for (int i = 0; i < /*tempSensorNum*/childsensors.length(); i++) {
		TemperatureSensor* tempSensor = (TemperatureSensor*)childsensors.get(i);
		
		// call dallasSensors.requestTemperatures() to issue a global temperature 
		// request to all devices on the bus
		logger.print(tag, "\n\t sensor: ");
		logger.print(tag, tempSensor->name);
		logger.print(tag, "\n\t index: ");
		logger.print(tag, i);
		logger.print(tag, "\n\t addr ");
		logger.print(tag, tempSensor->getPhisicalAddress());

		float oldTemperature = tempSensor->temperature;
		logger.print(tag, "\n\t old Temperature   is: ");
		logger.print(tag, String(oldTemperature));

		float dallasTemperature = pDallasSensors->getTempC(tempSensor->sensorAddr);
		logger.print(tag, "\n\t dallas Temperature   is: ");
		logger.print(tag, String(dallasTemperature));
		
		tempSensor->temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		temperatureSensors[i].temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		logger.print(tag, "\n\t rounded Temperature  is: ");
		logger.print(tag, String(tempSensor->temperature));

		// se è cambiata almeno una temperatura ritorna true
		if (oldTemperature != tempSensor->temperature)
			res = true;

		if (avTempCounter < avTempsize) {
			avTemp[avTempCounter] = tempSensor->temperature;
			avTempCounter++;
		}
		else {
			for (int i = 0; i < avTempCounter - 1; i++)
			{
				avTemp[i] = avTemp[i + 1];
			}
			avTemp[avTempCounter - 1] = tempSensor->temperature;
		}
		float average = 0.0;
		for (int i = 0; i < avTempCounter; i++) {
			average += avTemp[i];
		}
		average = average / (avTempCounter);
		tempSensor->avTemperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		temperatureSensors[i].avTemperature = ((int)(average * 100 + .5) / 100.0);

		logger.print(tag, "\n\tAverage temperature  is: ");
		logger.print(tag, String(tempSensor->avTemperature));
		logger.print(tag, "\n");
	}
	
	if(res)
		logger.print(tag, "\n\n\t --temperatura cambiata");
	else
		logger.print(tag, "\n\n\t >>readTemperatures - temperatura non cambiata");
	
	return res;
}

String OnewireSensor::getJSONFields(int jsontype)
{
	logger.print(tag, "\n\t >>OnewireSensor::getJSONFields");

	String json = "";

	/*json += ",\"childsensors\":[";

	for (int i = 0; i < childsensors.length(); i++) {


		if (i > 0)
			json += ",";

		json += ((Sensor*)childsensors.get(i))->getJSON();
	}

	json += "]";*/

	logger.print(tag, "\n\t <<OnewireSensor::getJSONFields json=" + json);
	return json;
}

void OnewireSensor::addTemperatureSensorsFromJson(JSON sensorJson) {
	// QUETSA CHI LA CHIAMA?? DA ELIMINARE

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
