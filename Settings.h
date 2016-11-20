#pragma once

#include <Arduino.h>
#include "Actuator.h"
#include "Sensor.h"
#include "List.h"
#include "HeaterActuator.h"

#include <OneWire.h>
#include <DallasTemperature.h>



class Settings
{	
private:
	String tag;

public:
	static const int boardnamelen = 30;
	static const int servernamelen = 30;

	Settings();
	~Settings();
	String getSensorsStatusJson();
	String getActuatorsStatusJson();

	char networkSSID[32];// = "ssid";
	char networkPassword[96];// = "password";
	int localPort = 80;
	char servername[servernamelen];
	int serverPort;
	char boardname[boardnamelen];
	unsigned char MAC_array[6];
	char MAC_char[18];
	int id = 0; // inizializzato a zero perchè viene impostato dalla chiamata a registershield

	

	String localIP;

	//SimpleList<Actuator> actuatorList;
	List sensorList;
	List ActuatorList;
	
	void addOneWireSensors(String sensorNames);
	void addActuators();
	void readTemperatures();

	bool temperatureChanged = false; // indica se la temperatura è cambiata dall'ultima chiamata a flash()

	HeaterActuator hearterActuator;

private:
	

};

