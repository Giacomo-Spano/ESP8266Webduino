#include "Settings.h"
#include "DS18S20Sensor.h"
#include "Logger.h"

extern uint8_t OneWirePin;
extern OneWire oneWire;
extern DallasTemperature sensors;

extern Logger logger;

Settings::Settings()
{
	tag = "Settings";

	serverPort = 8080;

	//addOneWireSensors();

	//Actuator* pActuator = new Actuator();
	//actuatorList.push_back(*pActuator);

}


Settings::~Settings()
{
}

void Settings::addOneWireSensors(String sensorNames) {

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

void Settings::addActuators() {

	logger.println(tag, "addActuator...\n\r");
	ActuatorList.init();
}


void Settings::readTemperatures() {

	logger.println(tag, "\n\t--START readTemperatures---\n");
	sensorList.show();
	float oldTemperature;
	for (int i = 0; i < sensorList.count; i++) {
		DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);		
		logger.print(tag, "\n\n\t readTemperatures for sensor ");
		logger.print(tag, pSensor->sensorname);
		logger.print(tag, " addr: ");
		logger.print(tag, pSensor->getSensorAddress());
		oldTemperature = pSensor->avTemperature;
		
		sensorList.show();
		pSensor->readTemperature();
		
		if (oldLocalAvTemperature != pSensor->avTemperature)
			temperatureChanged = true;

		sensorList.show();
	}

	logger.print(tag, "\n\t---END readTemperatures---");
}

