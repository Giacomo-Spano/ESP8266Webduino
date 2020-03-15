// 
// 
// 

#include "OnewireSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"
#include "TemperatureSensor.h"
#include "SensorFactory.h"

Logger OnewireSensor::logger;
String OnewireSensor::tag = "OnewireSensor";

OnewireSensor::OnewireSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	logger.print(tag, "\n");
	logger.println(tag, F(">>OnewireSensor"));

	checkStatus_interval = 60000;
	lastCheckStatus = 0;

	type = "onewiresensor";
	// beginTemperatureSensors deve essere chiamata nel costruttore per inizializzare
	// la var tempSensorNum. 
	beginTemperatureSensors();

#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266

	logger.println(tag, F("<<OnewireSensor"));
}

void OnewireSensor::loadChildren(JsonArray& jsonarray) {
	//logger.println(tag, ">>loadChildren jarray=" + jarray.toString());
	//childsensors.show();

	//String jsonChild = jarray.getFirst();
	int current = 0;
	for (int i=0; i < jsonarray.size(); i++) {
	//while (jsonChild != "") {

		Sensor* sensor = (Sensor*)childsensors.get(current++);
		if (sensor == nullptr) break;

		//JSONObject json(jsonChild);
		JsonObject& json = jsonarray[i];
		if (json.containsKey("name")) {
			String name = json["name"];
			//logger.print(tag, "\n\t name=" + name);
			sensor->sensorname = name;
		}
		if (json.containsKey("id")) {
			int sensorid = json["id"];
			//logger.print(tag, "\n\t sensorid=" + sensorid);
			sensor->sensorid = sensorid;
		}
		//jsonChild = jarray.getNext();
	}

	//childsensors.show();
	//logger.println(tag, "<<loadChildren");
}

OnewireSensor::~OnewireSensor()
{
	//logger.print(tag, "\n");
	//logger.println(tag, "~OnewireSensor");
}

void OnewireSensor::init()
{

}

void OnewireSensor::beginTemperatureSensors()
{
	// questa funz può essere chiamata solo dopo
	// aver inizializzato pin
	logger.print(tag, F("\n"));
	logger.print(tag, F("\n\t >>beginTemperatureSensors pin="));
	logger.print(tag, String(pin));
	oneWirePtr = new OneWire(pin);
	pDallasSensors = new DallasTemperature(oneWirePtr);
	pDallasSensors->begin();

	uint8_t _address[8];
	tempSensorNum = 0;

	//SensorFactory factory;
	childsensors.clear();

	logger.print(tag, F("\n\t search for 1-Wire devices....."));
	while (oneWirePtr->search(_address) && tempSensorNum < OnewireSensor::maxTempSensors) {
		
#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266


		// cerca il prossimo sensore di temperatura reali attaccato allo stesso pin
		logger.print(tag, F("\n\tFound \'1-Wire\' device with _address:"));
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
			logger.print(tag, F("\n\t CRC is not valid!"));
			return;
		}

		// crea un nuovo TemperatreSensor assegnadogli l'address fisico e dei valori
		// di nome temporanei
		// L'eventuale nome personalizzzato
		// è caricato successivamente dalla loadChildren. Non si può fare qui!

		int id = tempSensorNum + 1;
		String name = "sensoretemperatura" + String(id);
		String subaddress = address + "." + id;
		TemperatureSensor* child = (TemperatureSensor*)SensorFactory::createSensor(0,"temperaturesensor", pin, true, subaddress, name);
		if (child != nullptr) {
			child->id = id;
			for (int i = 0; i < 8; i++) {
				child->sensorAddr[i] = _address[i];
			}

			childsensors.add(child);
			tempSensorNum++;

			//childsensors.show();
		}
	}
	oneWirePtr->reset_search();

	if (tempSensorNum == 0)
		logger.println(tag, F("NO TEMPERATURE SENSOR FOUND\n"));
	//childsensors.show();
	logger.println(tag, F("<<beginTemperatureSensors\n"));
}

/*float OnewireSensor::getTemperature(int index) {
	if (index < 0 || index > maxTempSensors)
		return -99;

	return temperatureSensors[index].temperature;
}

float OnewireSensor::getAvTemperature(int index) {
	if (index < 0 || index > maxTempSensors)
		return -99;

	return temperatureSensors[index].avTemperature;
}*/

bool OnewireSensor::readTemperatures() {

	logger.print(tag, F("\n\n\t >>readTemperatures"));
	// questa funzione ritorna true se è cambiata almeno uan tempertura
	int res = false; // 

	pDallasSensors->requestTemperatures(); // Send the command to get temperatures

	//logger.print(tag, F("\n\t childsensors.length(): ") + String(childsensors.length()));
	for (int i = 0; i < /*tempSensorNum*/childsensors.size(); i++) {
		TemperatureSensor* tempSensor = (TemperatureSensor*)childsensors.get(i);
		
		// call dallasSensors.requestTemperatures() to issue a global temperature 
		// request to all devices on the bus
		logger.print(tag, F("n\t sensor: "));
		logger.print(tag, tempSensor->name);
		logger.print(tag, F("\n\t index: "));
		logger.print(tag, i);
		logger.print(tag, F("\n\t addr "));
		logger.print(tag, tempSensor->getPhisicalAddress());

		float oldTemperature = tempSensor->temperature;
		logger.print(tag, F("\n\t old Temperature   is: "));
		logger.print(tag, String(oldTemperature));

		float dallasTemperature = pDallasSensors->getTempC(tempSensor->sensorAddr);
		logger.print(tag, F("\n\t dallas Temperature   is: "));
		logger.print(tag, String(dallasTemperature));
		
		tempSensor->temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		//temperatureSensors[i].temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		logger.print(tag, F("\n\t rounded Temperature  is: "));
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
		//temperatureSensors[i].avTemperature = ((int)(average * 100 + .5) / 100.0);

		logger.print(tag, F("\n\tAverage temperature  is: "));
		logger.print(tag, String(tempSensor->avTemperature));
		logger.print(tag, F("\n"));
	}
	
	if(res)
		logger.print(tag, F("\n\n\t --temperatura cambiata"));
	else
		logger.print(tag, F("\n\n\t >>readTemperatures - temperatura non cambiata"));
	
	return res;
}

void OnewireSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	//json["codetype"] = codetype;
}

/*String OnewireSensor::getJSONFields() {

	//logger.println(tag, ">>getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field

	//logger.println(tag, "<<getJSONField");
	return json;
}*/

/*void OnewireSensor::addTemperatureSensorsFromJson(JSON sensorJson) {
	// QUETSA CHI LA CHIAMA?? DA ELIMINARE

	if (sensorJson.has("childsensors")) {
		String names = "";
		String str = sensorJson.jsonGetArrayString("childsensors");
		logger.print(tag, "\n\t str=" + str);
		JSONArray jArrayTempSensor(str);
		String tempSensor = jArrayTempSensor.getFirst();
		int n = 0;// tempSensorNum = 0;
		while (!tempSensor.equals("") && n < OnewireSensor::maxTempSensors) {
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
}*/

bool OnewireSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	//logger.println(tag, "\n\t currMillis="+String(currMillis) + "timeDiff=" + String(timeDiff));
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, F("\n\n"));
		//logger.println(tag, ">>checkStatusChange()::checkTemperatures timeDiff:" + String(timeDiff) + " checkStatus_interval:" + String(checkStatus_interval));
		lastCheckStatus = currMillis;
		bool temperatureChanged = readTemperatures();
		if (temperatureChanged)
			logger.println(tag, F("temperatura cambiata"));
		else
			logger.println(tag, F("temperatura NON cambiata"));
		logger.print(tag, F("\n\n"));
		logger.println(tag, F("<<checkStatusChange()::checkTemperatures"));
		return temperatureChanged;
	}

	return false;
}
