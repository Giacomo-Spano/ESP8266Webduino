#pragma once

#include <Arduino.h>
#include "Actuator.h"
#include "Sensor.h"
#include "List.h"
#include "HeaterActuator.h"

#include <OneWire.h>
#include <DallasTemperature.h>



class Shield
{	
private:
	String tag;

	const int checkTemperature_interval = 60000;
	unsigned long lastCheckTemperature = 0;//-flash_interval;

public:
	static const int boardnamelen = 30;
	static const int servernamelen = 30;
	
	static const int maxIoDevices = 10; // queste sono le porte
	static const int maxIoDeviceTypes = 3;  // mweusti sono i tipi di device
	static const int ioDevices_Disconnected = 0;
	static const int ioDevices_Heater = 1;
	static const int ioDevices_OneWireSensors = 2;
	//static const char* ioDevicesTypeNames[];// = { "disconnected","Heater","OneWire sensors" };

	static int id;// = 0; // inizializzato a zero perchè viene impostato dalla chiamata a registershield
	static char servername[servernamelen];
	static int serverPort;
	static int ioDevices[maxIoDevices];

	static int getShieldId() { return id; } // static member function

	static String getIoDevicesTypeName(int type)
	{
		switch (type) {
		case 0:
			return "disconnected";
		case 1:
			return "Heater";
		case 2:
			return "OneWire sensors";
		}

		return "empty";
	}

	static int getMaxIoDevices()
	{
		return maxIoDevices;
	}

	static int getMaxIoDeviceTypes()
	{
		return maxIoDeviceTypes;
	}

	static String getServerName()
	{
		return String(servername);
	}

	static int getServerPort()
	{
		return serverPort;
	}

	static int getIODevice(int port)
	{
		if (port > 0 && port < maxIoDevices)
			return ioDevices[port];
		else return ioDevices_Disconnected;
	}

	static void setIODevice(int port, int ioDevice)
	{
		ioDevices[port] = ioDevice;
	}

	
	Shield();
	~Shield();
	String getSensorsStatusJson();
	String getActuatorsStatusJson();
	void checkActuatorsStatus();
	void checkSensorsStatus();

	char networkSSID[32];// = "ssid";
	char networkPassword[96];// = "password";
	int localPort = 80;
	
	char boardname[boardnamelen];
	unsigned char MAC_array[6];
	char MAC_char[18];
	
	

	String localIP;

	//SimpleList<Actuator> actuatorList;
	List sensorList;
	List ActuatorList;
	
	void addOneWireSensors(String sensorNames);
	void addActuators();
	void checkTemperatures();

	bool temperatureChanged = false; // indica se la temperatura è cambiata dall'ultima chiamata a flash()

	boolean heaterConnected = false;
	HeaterActuator hearterActuator;

private:
	

};

