#pragma once

#include <Arduino.h>
#include "Actuator.h"
#include "Sensor.h"
#include "List.h"
#include "HeaterActuator.h"
#include "JSON.h"
#include <OneWire.h>
#include <DallasTemperature.h>



class Shield
{	
private:
	static String tag;
	static Logger logger;

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
	//static const char* ioDevicesTypeNames[];// = { "disconnected","Heater","OneWire dallasSensors" };

	static int id;// = 0; // inizializzato a zero perchè viene impostato dalla chiamata a registershield
	static char servername[servernamelen];
	static int serverPort;
	static int ioDevices[maxIoDevices];
	static uint8_t oneWirePin;
	static uint8_t heaterPin; // pin rele heater
	static bool heaterEnabled;

	static int getShieldId() { return id; } // static member function

	static String getIoDevicesTypeName(int type)
	{
		switch (type) {
		case 0:
			return "disconnected";
		case 1:
			return "Heater";
		case 2:
			return "OneWire dallasSensors";
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

	static uint8_t getHeaterPin()
	{
		return heaterPin;
	}

	static String getStrPinFromPin(uint8_t)
	{
		if (heaterPin == D1)
			return "D1";
		if (heaterPin == D2)
			return "D2";
		if (heaterPin == D3)
			return "D3";
		if (heaterPin == D4)
			return "D4";
		if (heaterPin == D5)
			return "D5";
		if (heaterPin == D6)
			return "D6";
		if (heaterPin == D7)
			return "D7";
	}

	static void setHeaterPin(int pin)
	{
		logger.print(tag, "\n\t>>setHeaterPin: " + String(pin));
		heaterPin = pin;
	}

	static uint8_t getOneWirePin()
	{
		return oneWirePin;
	}

	static void setOneWirePin(uint8_t pin)
	{
		oneWirePin = pin;
	}

	static bool getHeaterEnabled()
	{
		return heaterEnabled;
	}

	static void setHeaterEnabled(bool enabled)
	{
		logger.print(tag, "\n\t>>setHeaterEnabled: " + String(enabled));
		heaterEnabled = enabled;
		if (heaterEnabled)
			logger.print(tag, " true");
		else
			logger.print(tag, " false");
		
	}

	
	Shield();
	~Shield();
	String getSensorsStatusJson();
	String getActuatorsStatusJson();

	String getHeaterStatusJson();

	void checkActuatorsStatus();
	void checkSensorsStatus();
	String sendCommand(String jsonStr);

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

	//boolean heaterConnected = false;
	HeaterActuator hearterActuator;

protected:
	String heaterSettingsCommand(JSON json);
	

};

