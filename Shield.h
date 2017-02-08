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

public:
	HeaterActuator hearterActuator;

	static const int shieldNameLen = 30;
	static const int serverNameLen = 30;
	static const char networkSSIDLen = 32;// = "ssid";
	static const char networkPasswordLen = 96;// = "password";

	static const int maxIoDevices = 10; // queste sono le porte
	static const int maxIoDeviceTypes = 3;  // mweusti sono i tipi di device
	static const int ioDevices_Disconnected = 0;
	static const int ioDevices_Heater = 1;
	static const int ioDevices_OneWireSensors = 2;

	static int id;// = 0; // inizializzato a zero perchè viene impostato dalla chiamata a registershield
	static int ioDevices[maxIoDevices];
	static bool heaterEnabled;
	static uint8_t oneWirePin;
	static uint8_t heaterPin; // pin rele heater
	static String powerStatus; // power
	static String lastRestartDate;
	

private:
	static String tag;
	static Logger logger;
	const int checkTemperature_interval = 60000;
	unsigned long lastCheckTemperature = 0;//-flash_interval;
	

protected:
	String sendHeaterSettingsCommand(JSON json);
	String sendShieldSettingsCommand(JSON jsonStr);
	String sendPowerCommand(JSON jsonStr);
	String sendRegisterCommand(JSON jsonStr);
	String sendResetCommand(JSON jsonStr);
	bool temperatureChanged = false; // indica se la temperatura è cambiata dall'ultima chiamata a flash()
	//SimpleList<Actuator> actuatorList;
	
	static int localPort;
	static String networkSSID;// [32];// = "ssid";
	static String networkPassword;//[96];// = "password";
	static String serverName;//[serverNameLen];
	static int serverPort;
	static String shieldName;

public:

	List sensorList;
	List ActuatorList;

	Shield();
	~Shield();
	String getSensorsStatusJson();
	String getActuatorsStatusJson();
	String getHeaterStatusJson();
	String getSettingsJson();
	void checkActuatorsStatus();
	void checkSensorsStatus();
	String sendCommand(String jsonStr);	
	
	
	unsigned char MAC_array[6];
	char MAC_char[18];
	String localIP;
	
	void addOneWireSensors(String sensorNames);
	void addActuators();
	void checkTemperatures();

	static int getShieldId() { return id; } // static member function

	static String getLastRestartDate() { return lastRestartDate; }

	static void setLastRestartDate(String date) { lastRestartDate = date; }

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
	
	static int getServerPort()
	{
		return serverPort;
	}

	static void setServerPort(int port)
	{		
		logger.print(tag, "\n\t>>setServerPort");
		serverPort = port;
		logger.print(tag, "\n\t serverPort=" + String(serverPort));
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

	static String getStrHeaterPin()
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
	
	static int getLocalPort()
	{
		return localPort;
	}

	static void setLocalPort(int port)
	{
		logger.print(tag, "\n\t >>setLocalPort");
		localPort = port;
		logger.print(tag, "\n\t localPort="+ String(localPort));
	}

	static String getNetworkSSID()
	{
		return String(networkSSID);
	}

	static void setNetworkSSID(String ssid)
	{
		logger.print(tag, "\n\t>>setNetworkSSID");
		networkSSID = ssid;
		logger.print(tag, "\n\tnetworkSSID=");
		logger.print(tag, networkSSID);
	}

	static String getNetworkPassword()
	{

		return String(networkPassword);
	}

	static void setNetworkPassword(String password)
	{
		logger.print(tag, "\n\t>>setNetworkPassword");
		networkPassword = password;
		logger.print(tag, "\n\tnetworkPassword=");
		logger.print(tag, networkPassword);
	}

	static String getServerName()
	{
		return String(serverName);
	}

	static void setServerName(String name)
	{
		logger.print(tag, "\n\t>>setServerName");
		serverName = name;
		logger.print(tag, "\n\tserverName=");
		logger.print(tag, serverName);
	}
	
	static String getShieldName()
	{
		return String(shieldName);
	}

	static void setShieldName(String name)
	{
		logger.print(tag, "\n\t>>setShieldName");
		shieldName = name;
		logger.print(tag, "\n\tshieldName=");
		logger.print(tag, shieldName);
	}

	static String getPowerStatus()
	{
		return powerStatus;
	}

	static void setPowerStatus(String status)
	{
		logger.print(tag, "\n\t>>setpowerStatus");
		powerStatus = status;
		logger.print(tag, "\n\t powerSatus=" + powerStatus);
	}

};

