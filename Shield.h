#pragma once

#include <Arduino.h>
#include <string.h>
//#include "Actuator.h"
#include "Sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPDisplay.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include "TFTDisplay.h"
#endif
#include "DoorSensor.h"
#include "OnewireSensor.h"
#include "HornSensor.h"
#include <SimpleList.h>
#include <LinkedList.h>

#include "LoRaGateway.h"


class Shield
{	
protected:
	//static String networkSSID;
	//static String networkPassword;
	int localPort;
	String serverName;//[serverNameLen];
	int serverPort;
	String mqttServer;
	int mqttPort;
	String shieldName;
	bool nexiondisplay;
	bool oleddisplay;
	bool loragatewayEnabled;

	bool mqttMode;
	bool configMode;
	bool resetSettings;
	String mqttUser;
	String mqttPassword;
	String rebootreason;
	String loraMessage;

	LoRaGateway* loraGateway = nullptr;

public:
	int freeMemory = 0;
	int checkHealth_timeout = 86400000 / 2;  //12 ore
	unsigned long lastCheckHealth;
	String swVersion;
	//static const int shieldNameLen = 30;
	//static const int serverNameLen = 30;
	//static const char networkSSIDLen = 32;// = "ssid";
	//static const char networkPasswordLen = 96;// = "password";

	bool settingFromServerReceived = false;
	//bool settingFromServerRequested = false;
	//unsigned long settingRequestedTime;
	
	unsigned long settingsRequest_interval = 1 * 60 * 1000;
	const int settingsRequest_timeout = 1 * 60 * 1000;
	unsigned long lastSettingRequest = 0;
	bool settingsNeedToBeUpdated = true;
	bool settingsRequestInprogress = false;
	
	const int timeSync_interval = 5*60*1000;// *12;// 60 secondi * 15 minuti
	const int timeRequest_timeout = 1 * 60 * 1000;
	unsigned long lastTimeRequest = 0;
	bool timeNeedToBeUpdated = true;
	bool timeRequestInprogress = false;
	unsigned long lastTimeUpdate = 0; // usato dall'orologio per aggiornare il display ogni secondo
	int id;// = 0; // inizializzato a zero perch� viene impostato dalla chiamata a registershield
	String powerStatus; // power
	String lastRestartDate;
	//static const int maxSensorNum = 10;
	bool loadSensors(JsonObject& json);
	void readSensorFromFile();
	bool writeSensorToFile(JsonObject& json);
	void parseMessageReceived(String topic, String message);
	void drawString(int x, int y, String txt, int size, int color);
	void drawDateTime();
	void drawLora();
	void drawStatus();
	void drawMemory();
	void drawEvent();
	void drawSensorsStatus();
	void drawSWVersion();
	void clearScreen();
	void invalidateDisplay();

	void checkTimeUpdateStatus();
	void checkSettingResquestStatus();

	void readConfig();
	void writeConfig();
	void readRebootReason();
	void writeRebootReason();

	void setLoRaGateway(bool enable, String address, bool serverenabled);
	bool sendLoRaMessage(String payload);

	void setFreeMem(int mem);

private:
	static String tag;
	static Logger logger;

	String status;
	String shieldEvent;
	ESPDisplay espDisplay;
	
	String oldDate;

protected:

	bool onShieldSettingsCommand(JsonObject& json);
	bool onPowerCommand(JsonObject& json);
	void checkSensorsStatus();	

	
	ESPDisplay display;
#ifdef ESP8266
	TFTDisplay tftDisplay;
#endif

public:

	LinkedList<Sensor*> sensors = LinkedList<Sensor*>();
	
	Shield();
	~Shield();
	void init();
	virtual void getJson(JsonObject& json);
	void checkStatus();	
	void setStatus(String txt);
	void setEvent(String txt);
	bool receiveCommand(String jsonStr);
	bool requestTime();
	bool requestSettingsFromServer();
	String localIP;
	void clearAllSensors();
	Sensor* getSensorFromAddress(String addr);
	Sensor* getSensorFromId(int id);

	int getShieldId() {
		return id; 
	} 

	void setShieldId(int shieldid) { 
		id = shieldid;
	}
	String getLastRestartDate() { return lastRestartDate; }
	void setLastRestartDate(String date) { lastRestartDate = date; }

	String getMACAddress()
	{
#ifdef ESP8266
		return WiFi.macAddress();
#else
		uint8_t baseMac[6];
		// Get MAC address for WiFi station
		esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
		char baseMacChr[18] = { 0 };
		sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

		return String(baseMacChr);
#endif
	}

	String getSWVersion()
	{
		return swVersion;
	}
		
	int getServerPort()
	{
		return serverPort;
	}

	void setServerPort(int port)
	{		
		logger.print(tag, "\n\t >>setServerPort");
		serverPort = port;
		logger.print(tag, "\n\t <<serverPort=" + String(serverPort));
	}

	int getNextionDisplay()
	{
		return nexiondisplay;
	}

	void setNextionDisplay(bool enable)
	{
		logger.print(tag, "\n\t >>setnextionDisplay");
		nexiondisplay = enable;
		logger.print(tag, "\n\t <<setnextionDisplay=" + Logger::boolToString(nexiondisplay));
	}

	bool getOledDisplay()
	{
		return oleddisplay;
	}

	void setOledDisplay(bool enable)
	{
		logger.print(tag, "\n\t >>setOledDisplay");
		oleddisplay = enable;
		logger.print(tag, "\n\t <<setOledDisplay=" + Logger::boolToString(oleddisplay));
	}

	bool getLoRaGateway()
	{
		return loragatewayEnabled;
	}

	bool getLoRaGatewayServer()
	{
		if (loraGateway != nullptr)
			return loraGateway->getGatewayServer();
		else
			return false;
	}

	String getLoRaGatewayTargetAddress() {
		if (loraGateway != nullptr)
			return loraGateway->getTargetAddress();
		else
			return "";
	}

	

	static String getStrPin(uint8_t pin)
	{
#ifdef ESP8266
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
#ifdef D9
		if (pin == D9)
			return "D9";
#endif
#ifdef D10
		if (pin == D10)
			return "D10";
#endif
		else
#endif // ESP8266
			return "";
	}

	static uint8_t pinFromStr(String str)
	{
		logger.print(tag, "\n\t >> pinFromStr strPin=");
		logger.print(tag, str);
//#ifdef ESP8266
		if (str.equalsIgnoreCase("D0"))
			return D0;
		if (str.equals("D1"))
			return D1;
		if (str.equalsIgnoreCase("D2"))
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
#ifdef D9
		if (str.equals("D9"))
			return D9;
#endif
#ifdef D10
		if (str.equals("D10"))
			return D10;
#endif
		/*else
//#else
		if (str.equals("D0"))
				return 0;
		if (str.equals("D1"))
			return 1;
		if (str.equals("D2"))
			return 2;
		if (str.equals("D3"))
			return 3;
		if (str.equals("D4"))
			return 4;
		if (str.equals("D5"))
			return 5;
		if (str.equals("D6"))
			return 6;
		if (str.equals("D7"))
			return 7;
		if (str.equals("D8"))
			return 8;
		if (str.equals("D9"))
			return 9;
		if (str.equals("D10"))
			return 10;
		else*/
//#endif // ESP8266
			logger.print(tag, "\n\t PIN NOT FOUND");
			return 0;
	}

	
	int getLocalPort()
	{
		return localPort;
	}

	void setLocalPort(int port)
	{
		logger.print(tag, "\n\t >>setLocalPort");
		localPort = port;
		logger.print(tag, "\n\t >> localPort="+ String(localPort));
	}

	String getServerName()
	{
		return String(serverName);
	}

	void setServerName(String name)
	{
		logger.print(tag, "\n\t >>setServerName");
		serverName = name;
		logger.print(tag, "\n\t serverName=");
		logger.print(tag, serverName);
	}
	
	String getShieldName()
	{
		return String(shieldName);
	}

	void setShieldName(String name)
	{
		logger.print(tag, "\n\t >>setShieldName");
		shieldName = name;
		logger.print(tag, "\n\t<< shieldName=");
		logger.print(tag, shieldName);
	}

	String getPowerStatus()
	{
		return powerStatus;
	}

	void setPowerStatus(String status)
	{
		logger.print(tag, "\n\t>>setpowerStatus");
		powerStatus = status;
		logger.print(tag, "\n\t<< powerSatus=" + String(powerStatus));
	}

	bool getMQTTmode()
	{
		return mqttMode;
	}

	void setMQTTMode(bool enabled)
	{
		logger.print(tag, "\n\t>> setMQTTMode");
		mqttMode = enabled;
		logger.print(tag, "\n\t<< setMQTTMode=" + String(mqttMode));
	}

	bool getConfigMode()
	{
		return configMode;
	}

	void setConfigMode(bool enabled)
	{
		logger.print(tag, "\n\t>> setConfigMode");
		configMode = enabled;
		logger.print(tag, "\n\t<< setConfigMode=" + String(configMode));
	}

	bool getResetSettings()
	{
		return resetSettings;
	}

	void setResetSettings(bool enabled)
	{
		logger.print(tag, "\n\t>> setResetSettings");
		resetSettings = enabled;
		logger.print(tag, "\n\t<< setResetSettings=" + String(resetSettings));
	}

	String getMQTTServer()
	{
		return /*"192.168.1.21";////*/  mqttServer /*serverName*/;
	}

	void setMQTTServer(String server)
	{
		logger.print(tag, "\n\t>> setMQTTServer");
		mqttServer = server;
		logger.print(tag, "\n\t<< setMQTTServer=" + String(mqttServer));
	}

	String getMQTTUser()
	{
		return mqttUser;
	}

	void setMQTTUser(String user)
	{
		logger.print(tag, "\n\t>> setMQTTUser");
		mqttUser = user;
		logger.print(tag, "\n\t<< setMQTTUser=" + String(mqttUser));
	}

	String getMQTTPassword()
	{
		return mqttPassword;
	}

	void setMQTTPassword(String password)
	{
		logger.print(tag, "\n\t>> setMQTTPassword");
		mqttPassword = password;
		logger.print(tag, "\n\t<< setMQTTPassword=" + String(mqttPassword));
	}


	int getMQTTPort()
	{
		return mqttPort;
	}

	void setMQTTPort(int port)
	{
		//logger.print(tag, "\n\t>> setMQTTPort");
		mqttPort = port;
		//logger.print(tag, "\n\t<< setMQTTPort=" + String(mqttPort));
	}

	String getRebootReason()
	{
		return rebootreason;
	}

	void setRebootReason(String reason)
	{
		//logger.print(tag, "\n\t>> setMQTTPort");
		rebootreason = reason;
		//logger.print(tag, "\n\t<< setMQTTPort=" + String(mqttPort));
	}
};

