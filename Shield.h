#pragma once

#include <Arduino.h>
#include <string.h>
#include "Actuator.h"
#include "Sensor.h"
#include "List.h"
#include "HeaterActuator.h"
#include "JSON.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPDisplay.h"
#include "TFTDisplay.h"
#include "DoorSensor.h"
#include "OnewireSensor.h"

class Shield
{	
protected:
	static String networkSSID;
	static String networkPassword;
	static int localPort;
	static String serverName;//[serverNameLen];
	static int serverPort;
	static String shieldName;

public:
	HeaterActuator hearterActuator;

	static String swVersion;
	static const int shieldNameLen = 30;
	static const int serverNameLen = 30;
	static const char networkSSIDLen = 32;// = "ssid";
	static const char networkPasswordLen = 96;// = "password";

	static const int maxIoDevices = 10; // queste sono le porte
	static const int maxIoDeviceTypes = 3;  // mweusti sono i tipi di device
	static const int ioDevices_Disconnected = 0;
	static const int ioDevices_Heater = 1;
	static const int ioDevices_OneWireSensors = 2;

	static int id;// = 0; // inizializzato a zero perch� viene impostato dalla chiamata a registershield
	static int ioDevices[maxIoDevices];
	static bool heaterEnabled;
	static bool temperatureSensorsEnabled;
	static uint8_t oneWirePin;
	static uint8_t heaterPin; // pin rele heater
	static String powerStatus; // power
	static String lastRestartDate;

	static const int maxSensorNum = 10;

	void drawString(int x, int y, String txt, int size, int color);
	void clearScreen();
	

private:
	static String tag;
	static Logger logger;
	

	const int checkTemperature_interval = 60000;
	const int checkDoorStatus_interval = 1000; // 1 secondo
	unsigned long lastCheckTemperature = 0;//-flash_interval;
	unsigned long lastCheckDoorStatus = 0;//-flash_interval;
	String oldDate;
	

protected:
	String sendHeaterSettingsCommand(JSON json);
	String sendUpdateSensorListCommand(JSON json);
	String sendShieldSettingsCommand(JSON jsonStr);
	String sendPowerCommand(JSON jsonStr);
	String sendRegisterCommand(JSON jsonStr);
	String sendResetCommand(JSON jsonStr);
	bool temperatureChanged = false; // indica se la temperatura � cambiata dall'ultima chiamata a flash()
	//SimpleList<Actuator> actuatorList;
	void checkActuatorsStatus();
	void checkSensorsStatus();	
	
	
	

	ESPDisplay display;
	TFTDisplay tftDisplay;

public:

	List sensorList;
	List ActuatorList;

	Shield();
	~Shield();
	void init();
	String getSensorsStatusJson();
	//String getTemperatureSensorsStatusJson();
	String getActuatorsStatusJson();
	String getHeaterStatusJson();
	String getSettingsJson();
	void checkStatus();	
	String getOneWireJson();
	String sendCommand(String jsonStr);		
	
	unsigned char MAC_array[6];
	char MAC_char[18];
	String localIP;
	
	void addSensor(Sensor* pSensor);
	void clearAllSensors();

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
		logger.print(tag, "\n\t >>setServerPort");
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
		if (heaterPin == D0)
			return "D0";
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
		if (heaterPin == D8)
			return "D8";
		if (heaterPin == D9)
			return "D9";
		if (heaterPin == D10)
			return "D10";
	}

	
	static String getStrOneWirePin()
	{
		if (oneWirePin == D0)
			return "D0";
		if (oneWirePin == D1)
			return "D1";
		if (oneWirePin == D2)
			return "D2";
		if (oneWirePin == D3)
			return "D3";
		if (oneWirePin == D4)
			return "D4";
		if (oneWirePin == D5)
			return "D5";
		if (oneWirePin == D6)
			return "D6";
		if (oneWirePin == D7)
			return "D7";
		if (oneWirePin == D8)
			return "D8"; 
		if (oneWirePin == D9)
			return "D9"; 
		if (oneWirePin == D10)
			return "D10";
		else
			return "";
	}

	static String getStrPin(uint8_t pin)
	{
		if (pin == D0)
			return "D0";
		if (pin == D1)
			return "D1";
		if (pin == D2)
			return "D2";
		if (pin == D3)
			return "D3";
		if (pin == D4)
			return "D4";
		if (pin == D5)
			return "D5";
		if (pin == D6)
			return "D6";
		if (pin == D7)
			return "D7";
		if (pin == D8)
			return "D8";
		if (pin == D9)
			return "D9";
		if (pin == D10)
			return "D10";
		else
			return "";
	}

	static uint8_t pinFromStr(String str)
	{
		if (str.equals("D0"))
			return D0;
		if (str.equals("D1"))
			return D1;
		if (str.equals("D2"))
			return D2;
		if (str.equals("D3"))
			return D3;
		if (str.equals("D4"))
			return D4;
		if (str.equals("D5"))
			return D5;
		if (str.equals("D6"))
			return D6;
		if (str.equals("D7"))
			return D7;
		if (str.equals("D8"))
			return D8;
		if (str.equals("D9"))
			return D9;
		if (str.equals("D10"))
			return D10;
		else
			return 0;
	}


	static void setHeaterPin(int pin)
	{
		logger.print(tag, "\n\t>>setHeaterPin: " + String(pin));
		heaterPin = pin;
		//hearterActuator.setRelePin(pin);
	}

	static uint8_t getOneWirePin()
	{
		return oneWirePin;
	}

	static void setOneWirePin(uint8_t pin)
	{
		oneWirePin = pin;
		//DS18S20Sensor::setOneWirePin(pin);
	}

	static bool getHeaterEnabled()
	{
		logger.print(tag, "\n\t >>getHeaterEnabled: " + String(heaterEnabled));
		return heaterEnabled;
	}

	static void setHeaterEnabled(bool enabled)
	{
		logger.print(tag, "\n\t >>setHeaterEnabled: " + String(enabled));
		heaterEnabled = enabled;
		/*if (heaterEnabled)
			logger.print(tag, " true");
		else
			logger.print(tag, " false");*/
		
	}

	static bool getTemperatureSensorsEnabled()
	{
		logger.print(tag, "\n\t >>getTemperatureSensorsEnabled: " + String(temperatureSensorsEnabled));
		return temperatureSensorsEnabled;
	}

	static void setTemperatureSensorsEnabled(bool enabled)
	{
		logger.print(tag, "\n\t >>setTemperatureSensorsEnabled: " + String(enabled));
		temperatureSensorsEnabled = enabled;
		//DS18S20Sensor::setTemperatureSensorsEnabled(enabled);
		
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
		//return "TP-LINK-3BD796";			
		return String(networkSSID);
	}

	static void setNetworkSSID(String ssid)
	{
		//logger.print(tag, "\n\t >>setNetworkSSID: " + ssid);
		networkSSID = ssid;
		//logger.print(tag, "\n\t networkSSID="+ networkSSID);
	}

	static String getNetworkPassword()
	{

		return String(networkPassword);
		//return "giacomocasa";
	}

	static void setNetworkPassword(String password)
	{
		//logger.print(tag, "\n\t >>setNetworkPassword: " + password);
		networkPassword = password;
		//logger.print(tag, "\n\t networkPassword=" + networkPassword);
	}

	static String getServerName()
	{
		return String(serverName);
	}

	static void setServerName(String name)
	{
		logger.print(tag, "\n\t >>setServerName");
		serverName = name;
		logger.print(tag, "\n\t serverName=");
		logger.print(tag, serverName);
	}
	
	static String getShieldName()
	{
		return String(shieldName);
	}

	static void setShieldName(String name)
	{
		logger.print(tag, "\n\t >>setShieldName");
		shieldName = name;
		logger.print(tag, "\n\t shieldName=");
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

