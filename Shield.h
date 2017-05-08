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

	static bool mqttMode;

public:
	static String swVersion;
	static const int shieldNameLen = 30;
	static const int serverNameLen = 30;
	static const char networkSSIDLen = 32;// = "ssid";
	static const char networkPasswordLen = 96;// = "password";

	static int id;// = 0; // inizializzato a zero perch� viene impostato dalla chiamata a registershield
	static String powerStatus; // power
	static String lastRestartDate;

	static const int maxSensorNum = 10;

	void drawString(int x, int y, String txt, int size, int color);
	void clearScreen();

private:
	static String tag;
	static Logger logger;
	
	String oldDate;

protected:
	String onUpdateSensorListCommand(JSON& json);
	String onShieldSettingsCommand(JSON& json);
	String onPowerCommand(JSON& json);
	String sendRegister();
	String onResetCommand(JSON& json);
	String onRebootCommand(JSON& json);
	String sendUpdateSensorStatus();
	

	//bool temperatureChanged = false; // indica se la temperatura � cambiata dall'ultima chiamata a flash()
	void checkSensorsStatus();	
	
	ESPDisplay display;
	TFTDisplay tftDisplay;

public:

	List sensorList;
	
	Shield();
	~Shield();
	void init();
	String getJson();
	String getSensorsStatusJson();
	String getSettingsJson();
	void registerShield();
	void checkStatus();	
	String receiveCommand(String jsonStr);		
	
	static unsigned char MAC_array[6];
	static char MAC_char[18];
	String localIP;
	
	void addSensor(Sensor* pSensor);
	void clearAllSensors();
	Sensor* getSensorFromAddress(String addr);

	static int getShieldId() {
		return id; 
	} 

	static void setShieldId(int shieldid) { 
		id = shieldid;
	}
	static String getLastRestartDate() { return lastRestartDate; }
	static void setLastRestartDate(String date) { lastRestartDate = date; }

	static String getMACAddress()
	{
		return String(MAC_char);
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
		//return "Vodafone-34230543";
		return networkSSID;
		//return "Connectify-me";
	}

	static void setNetworkSSID(String ssid)
	{
		//logger.print(tag, "\n\t >>setNetworkSSID: " + ssid);
		networkSSID = ssid;
		//logger.print(tag, "\n\t networkSSID="+ networkSSID);
	}

	static String getNetworkPassword()
	{

		return networkPassword;
		//return "giacomocasa";
		//return "giacomo00";
		//return "52iw4mmkdmw4ftt";
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
		logger.print(tag, "\n\t<< shieldName=");
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
		logger.print(tag, "\n\t<< powerSatus=" + String(powerStatus));
	}

	static bool getMQTTmode()
	{
		return mqttMode;
	}

	static void setMQTTMode(bool enabled)
	{
		logger.print(tag, "\n\t>> setMQTTMode");
		mqttMode = enabled;
		logger.print(tag, "\n\t<< setMQTTMode=" + String(mqttMode));
	}

};

