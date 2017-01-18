#include "Shield.h"
#include "DS18S20Sensor.h"
#include "Logger.h"
#include "HeaterActuator.h"
#include "Command.h"

extern uint8_t OneWirePin;
extern OneWire oneWire;
extern DallasTemperature sensors;

extern Logger logger;

int Shield::id = 0; //// inizializzato a zero perchè viene impostato dalla chiamata a registershield

int Shield::serverPort = 8080;

char  Shield::servername[servernamelen];


int Shield::ioDevices[maxIoDevices] = { 0,0,0,0,0,0,0,0,0,0 };
//char* Shield::ioDevicesTypeNames[] = { "disconnected","Heater","OneWire sensors" };

Shield::Shield()
{
	tag = "Shield";

	serverPort = 8080;

	//addOneWireSensors();

	//Actuator* pActuator = new Actuator();
	//actuatorList.push_back(*pActuator);
	
}


Shield::~Shield()
{
}

String Shield::getSensorsStatusJson() {
	String json = "{";
	json += "\"id\":" + String(id);// shieldid
	json += ",\"sensors\":[";
	for (int i = 0; i < sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)sensorList.get(i);
		if (i != 0)
			json += ",";
		json += sensor->getJSON();
	}
	json += "]";
	json += "}";
	return json;
}

String Shield::getActuatorsStatusJson() {
	String json = "{";
	json += "\"id\":" + String(id);// shieldid
	json += ",\"actuators\":[";
	
	hearterActuator.getJSON();
	/*for (int i = 0; i < sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)sensorList.get(i);
		if (i != 0)
			json += ",";
		json += sensor->getJSON();
	}*/
	json += "]";
	json += "}";
	return json;
}

void Shield::checkActuatorsStatus()
{
	hearterActuator.checkStatus();	
}

void Shield::checkSensorsStatus()
{
	//logger.println(tag, F(">>checkSensorsStatus"));

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckTemperature;
	if (timeDiff > checkTemperature_interval) {

		lastCheckTemperature = currMillis;
		checkTemperatures();
		return;
	}


	//checkTemperatures();

	/*// se il sensore attivo è quello locale aggiorna lo stato
	// del rele in base alla temperatur del sensore locale
	if (!hearterActuator.sensorIsRemote())
		hearterActuator.updateReleStatus();

	Command command;
	if (temperatureChanged) {

		logger.println(tag, "SEND TEMPERATURE UPDATE - average temperature changed");
		logger.print(tag, "\n\toldLocalAvTemperature=");
		
		command.sendSensorsStatus(*this);
		temperatureChanged = false; // qui bisognerebbe introdiurre il controllo del valore di ritorno della send
										   // cokmmand ed entualmente reinviare
	}*/

	//logger.println(tag, F("<<checkSensorsStatus"));
	//return true;
}

void Shield::addOneWireSensors(String sensorNames) {

	logger.println(tag, "addOneWireSensors - discoverOneWireDevices...\n\r");
	sensors.begin();
	logger.println(tag, "Looking for 1-Wire devices...\n\r");

	sensorList.init();
	
	uint8_t address[8];
	int count = 0;
	while (oneWire.search(address)) {
		logger.println(tag, "\n\tFound \'1-Wire\' device with address:\n");
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
			logger.println(tag, "CRC is not valid!\n");
			return;
		}

		logger.print(tag, "\n\tsensorNames=");
		logger.print(tag, sensorNames);
		String name = "";
		int index = sensorNames.indexOf(";");
		if (index >= 0) {
			name = sensorNames.substring(0, index);
			if (index < sensorNames.length() - 1)
				sensorNames = sensorNames.substring(index + 1);
			else
				sensorNames = "";
		}
		else {
			name = "sensor" + String(count);
		}

		DS18S20Sensor* pSensorNew = new DS18S20Sensor;
		pSensorNew->sensorname = name/* + String(count)*/;

		for (int i = 0; i < 8; i++) {
			pSensorNew->sensorAddr[i] = address[i];
		}

		logger.print(tag, "\n\tpSensorNew->sensorAddr=");
		logger.print(tag, pSensorNew->getSensorAddress());

		count++;
		sensorList.add((Sensor*)pSensorNew);
		sensorList.show();

		
		String txt = "ADDDED sensor " + String(pSensorNew->sensorname) + "addr ";
		logger.print(tag, txt);
		logger.print(tag, pSensorNew->getSensorAddress());
		logger.print(tag, " end");
	}
	
	logger.println(tag, "\n\r\n\rThat's it.\r\n");
	oneWire.reset_search();
	sensorList.show();
}

void Shield::addActuators() {

	logger.println(tag, "addActuator...\n\r");
	ActuatorList.init();

	//HeaterActuator* pActuatorNew = new HeaterActuator();
	//pActuatorNew->sensorname = "riscaldamento";
}


void Shield::checkTemperatures() {
	
	logger.println(tag, ">>checkTemperatures---");

	//sensorList.show();
	float oldTemperature;
	for (int i = 0; i < sensorList.count; i++) {
		DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);		
		oldTemperature = pSensor->getTemperature();
		
		//sensorList.show();
		pSensor->readTemperature();
		
		if (oldTemperature != pSensor->getTemperature()) {
			temperatureChanged = true;
			logger.print(tag, "\ttemperatura cambiata");
			logger.print(tag, String(oldTemperature));
			logger.print(tag, "->");
			logger.print(tag, String(pSensor->getTemperature()));
		}

		// imposta la temperatura locale a quella del primo sensore (DA CAMBIARE)
		if (i == 0)
			hearterActuator.setLocalTemperature(pSensor->getTemperature());
		//sensorList.show();
	}

	// se il sensore attivo è quello locale aggiorna lo stato
	// del rele in base alla temperatur del sensore locale
	if (!hearterActuator.sensorIsRemote())
		hearterActuator.updateReleStatus();

	Command command;
	if (temperatureChanged) {

		logger.print(tag, "\n\n\tSEND TEMPERATURE UPDATE - temperature changed\n");
		//logger.print(tag, "\n\toldLocalTemperature=");

		command.sendSensorsStatus(*this);
		temperatureChanged = false; // qui bisognerebbe introdiurre il controllo del valore di ritorno della send
									// cokmmand ed entualmente reinviare
	}

	logger.println(tag, "<<checkTemperatures\n");
}

