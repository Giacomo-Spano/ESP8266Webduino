

#include "OnewireSensor.h"
#include "JSONArray.h"
#include "DoorSensor.h"
#include "TFTDisplay.h"
#include "HttpResponse.h"
#include "ESPWebServer.h"
#include "HttpRequest.h"
#include "ESP8266Webduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "wol.h"
#include "Logger.h"
#include "HttpHelper.h"
#include "JSON.h"
#include "Shield.h"
#include "Command.h"
#include "Program.h"
#include "Actuator.h"
#include <Time.h>
#include "TimeLib.h"
#include "POSTData.h"

//#include "Ucglib.h"

//#include "ESPDisplay.h"

//ESPDisplay display;
#ifdef dopop
// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Initialize the OLED display using Wire library
// D7 -> SDA
// D6 -> SCL
SSD1306  display(0x3c, D7, D6);
#endif

//#define Versione 0.92
int EPROM_Table_Schema_Version = 8;
//const char SWVERSION[] = "1.01";

Logger logger;
String sensorNames = "";

String tag = "Webduino";
const char *ssidAP = "ES8266";
const char *passwordAP = "thereisnospoon";
const char* ssid = "xxBUBBLES";
Shield shield;

/*const*/ char* ssidtest = "TP-LINK_3BD796";
/*const*/ char* passwordtest = "giacomocasa";

// EPROM
const byte EEPROM_ID = 0x98; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int TIMERTIME_ADDR = 1; // the EEPROM address used to store the pin
int epromversion = 0;
int addr = TIMERTIME_ADDR;

void initEPROM();
void readEPROM();
void readSensor();
extern void writeEPROM();
extern void resetEPROM();

// html page old style
//String showMain(HttpRequest request, HttpResponse response);
//String showHeater(HttpRequest request, HttpResponse response);
//String showSettings(HttpRequest request, HttpResponse response);

// POST request
String receiveCommand(HttpRequest request, HttpResponse httpResponse);
String setRemoteTemperature(HttpRequest request, HttpResponse httpResponse);
String showPower(HttpRequest request, HttpResponse response);
//String showRele(HttpRequest request, HttpResponse response);
//String showChangeSettings(HttpRequest request, HttpResponse response);
String softwareReset(HttpRequest request, HttpResponse response);

// GET request
String getJsonStatus(HttpRequest request, HttpResponse response);
String getJsonActuatorsStatus(HttpRequest request, HttpResponse response);
String getJsonHeaterStatus(HttpRequest request, HttpResponse response);
String getJsonSettings(HttpRequest request, HttpResponse response);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;

const int flash_interval = 30000;
unsigned long lastFlash = 0;//-flash_interval;
unsigned long lastSendLog = 0;
const int SendLog_interval = 10000;// 10 secondi
unsigned long lastTimeSync = 0;
const int timeSync_interval = 60000 * 15;// *12;// 60 secondi * 15 minuti

extern int __bss_end;
extern void *__brkval;

int sendRestartNotification = 0;
bool shieldRegistered = false; // la shield si registra all'inizio e tutte le volte che va offline

void resetEPROM() {

	logger.print(tag, "\n\n\t >>resetEPROM");

	byte hiByte;
	byte loByte;
	EEPROM.write(ID_ADDR, 0/*EEPROM_ID*/); // delete EEPROM_ID to indicate invalid data

	EEPROM.commit();
	logger.println(tag, "\n\t <<resetEPROM");
}

void writeEPROM() {

	logger.print(tag, "\n\n\t >>write EPROM");
		
	byte hiByte;
	byte loByte;
	addr = TIMERTIME_ADDR;
	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data

	// build version
	hiByte = highByte(EPROM_Table_Schema_Version);
	loByte = lowByte(EPROM_Table_Schema_Version);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);

	// ssid
	char networkSSID[Shield::networkSSIDLen];
	Shield::getNetworkSSID().toCharArray(networkSSID, sizeof(networkSSID));
	int res = EEPROM_writeAnything(addr, networkSSID);
	addr += res;
	logger.print(tag, "\n\t networkSSID = " + String(networkSSID));

	// password
	char networkPasswordBuffer[Shield::networkPasswordLen];
	Shield::getNetworkPassword().toCharArray(networkPasswordBuffer, sizeof(networkPasswordBuffer));
	res = EEPROM_writeAnything(addr, networkPasswordBuffer);
	addr += res;
	logger.print(tag, "\n\t networkPasswordBuffer = " + String(networkPasswordBuffer));

	// local port
	int port = Shield::getLocalPort();
	hiByte = highByte(port);
	loByte = lowByte(port);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	logger.print(tag, "\n\t port = " + String(port));

	// server name
	char serverNameBuffer[Shield::serverNameLen];
	Shield::getServerName().toCharArray(serverNameBuffer, sizeof(serverNameBuffer));
	res = EEPROM_writeAnything(addr, serverNameBuffer);
	addr += res;
	logger.print(tag, "\n\t serverNameBuffer = " + String(serverNameBuffer));

	// server port
	port = Shield::getServerPort();
	hiByte = highByte(port);
	loByte = lowByte(port);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	logger.print(tag, "\n\t port = " + String(port));

	// shieldName name
	char shieldNameBuffer[Shield::shieldNameLen];
	Shield::getShieldName().toCharArray(shieldNameBuffer, sizeof(shieldNameBuffer));
	res = EEPROM_writeAnything(addr, shieldNameBuffer);
	addr += res;
	logger.print(tag, "\n\t shieldNameBuffer = " + String(shieldNameBuffer));

	// sensor count
	logger.print(tag, "\n\n\t shield.sensorList.count=" + String(shield.sensorList.count));
	int count = shield.sensorList.count;
	hiByte = highByte(count);
	loByte = lowByte(count);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);

	bool lastSensorWasTemperature = false;
	for (int i = 0; i < shield.sensorList.count; i++) {
		Sensor* sensor = shield.sensorList.get(i);

		logger.print(tag, "\n\n\t sensor->sensorname=" + sensor->sensorname);
		char sensorNameBuffer[Sensor::sensorNameLen];
		sensor->sensorname.toCharArray(sensorNameBuffer, sizeof(sensorNameBuffer));
		res = EEPROM_writeAnything(addr, sensorNameBuffer);
		addr += res;

		logger.print(tag, "\n\t sensor->sensorTypeBuffer=" + sensor->type);
		char sensorTypeBuffer[Sensor::sensorTypeLen];
		sensor->type.toCharArray(sensorTypeBuffer, sizeof(sensorTypeBuffer));
		res = EEPROM_writeAnything(addr, sensorTypeBuffer);
		addr += res;

		logger.print(tag, "\n\t sensor->address=" + sensor->address);
		char sensorAddressBuffer[Sensor::sensorAddressLen];
		sensor->address.toCharArray(sensorAddressBuffer, sizeof(sensorAddressBuffer));
		res = EEPROM_writeAnything(addr, sensorAddressBuffer);
		addr += res;

		logger.print(tag, "\n\t sensor->pin=" + String(sensor->pin));
		hiByte = highByte(sensor->pin);
		loByte = lowByte(sensor->pin);
		EEPROM.write(addr++, hiByte);
		EEPROM.write(addr++, loByte);

		logger.print(tag, "\n\t sensor->enabled=" + String(sensor->enabled));
		EEPROM.write(addr++, sensor->enabled);

		if (sensor->type == "onewiresensor") {
			logger.print(tag, "\n\t onewire sensor");
			OnewireSensor* onewire = (OnewireSensor*)sensor;

			hiByte = highByte(onewire->tempSensorNum);
			loByte = lowByte(onewire->tempSensorNum);
			EEPROM.write(addr++, hiByte);
			EEPROM.write(addr++, loByte);
			logger.print(tag, "\n\t onewire->tempSensorNum=" + String(onewire->tempSensorNum));
			
			for (int k = 0; k < onewire->tempSensorNum; k++) {

				hiByte = highByte(onewire->temperatureSensors[k].id);
				loByte = lowByte(onewire->temperatureSensors[k].id);
				EEPROM.write(addr++, hiByte);
				EEPROM.write(addr++, loByte);
				logger.print(tag, "\n\t onewire->temperatureSensors[k].id=" + onewire->temperatureSensors[k].id);

				char sensorNameBuffer[Sensor::sensorNameLen];
				onewire->temperatureSensors[k].name.toCharArray(sensorNameBuffer, sizeof(sensorNameBuffer));
				res = EEPROM_writeAnything(addr, sensorNameBuffer);
				logger.print(tag, "\n\t onewire->temperatureSensors[k].name=" + onewire->temperatureSensors[k].name);
				addr += res;
			}
		}
	}

	EEPROM.commit();
	logger.println(tag, "\n\t <<write EPROM\n");
}

void readEPROM() {

	logger.print(tag, "\n\n\t >>read EPROM");

	byte hiByte;
	byte lowByte;
	addr = ID_ADDR;

	// check EPROM_ID
	byte dummy = EEPROM.read(addr++);
	logger.print(tag, "\n\t dummy=" + String(dummy));

	if (dummy != EEPROM_ID) { // reset EPROM_ID
		logger.print(tag, "\n\t INVALID DATA" + String(dummy));
		writeEPROM();
		return;
	}

	addr = TIMERTIME_ADDR;
	// epromversion
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	epromversion = word(hiByte, lowByte);
	logger.print(tag, "\n\t epromversion=" + String(epromversion));

	// ssid
	char networkSSIDBuffer[Shield::networkSSIDLen];
	int res = EEPROM_readAnything(addr, networkSSIDBuffer);
	logger.print(tag, "\n\t networkSSIDBuffer=" + String(networkSSIDBuffer));
	Shield::setNetworkSSID(String(networkSSIDBuffer)/*"TP-LINK_3BD796"*/);
	addr += res;

	// password
	char networkPasswordBuffer[Shield::networkPasswordLen];
	res = EEPROM_readAnything(addr, networkPasswordBuffer);
	logger.print(tag, "\n\t networkPasswordBuffer = " + String(networkPasswordBuffer));
	Shield::setNetworkPassword(String(networkPasswordBuffer)/*"giacomocasa"*/);
	addr += res;

	// local port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	int port = word(hiByte, lowByte);
	logger.print(tag, "\n\t port = ");
	logger.print(tag, String(port));
	Shield::setLocalPort(port);

	//server name
	char servernnameBuffer[Shield::serverNameLen];
	res = EEPROM_readAnything(addr, servernnameBuffer);
	logger.print(tag, "\n\t servernnameBuffer=" + String(servernnameBuffer));
	Shield::setServerName(String(servernnameBuffer));
	addr += res;
	
	// server port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	port = word(hiByte, lowByte);
	logger.print(tag, "\n\t port=" + String(port));
	Shield::setServerPort(port);
	
	// shield name
	char shieldNameBuffer[Shield::shieldNameLen];
	res = EEPROM_readAnything(addr, shieldNameBuffer);
	logger.print(tag, "\n\t shieldNameBuffer =" + String(shieldNameBuffer));
	Shield::setShieldName(String(shieldNameBuffer));
	addr += res;
	
	logger.print(tag, "\n\n\t <<read EPROM res=\n" + String(addr));
}

void readSensor() {

	logger.print(tag, "\n\n\t >>read Sensor");

	byte hiByte;
	byte lowByte;

	// sensor count
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	int sensorCount = word(hiByte, lowByte);
	logger.print(tag, "\n\t sensorCount=" + String(sensorCount));
	if (sensorCount < 0 || sensorCount > Shield::maxSensorNum) {
		sensorCount = 0;
	}
	//sensorCount = 0;
	shield.clearAllSensors(); // serve per inizializzare
	for (int i = 0; i < sensorCount; i++) {

		// read sensor data
		char sensorNameBuffer[Sensor::sensorNameLen];
		int res = EEPROM_readAnything(addr, sensorNameBuffer);
		String name = String(sensorNameBuffer);
		logger.print(tag, "\n\n\t sensorNameBuffer =" + name);
		addr += res;

		char sensorTypeBuffer[Sensor::sensorTypeLen];
		res = EEPROM_readAnything(addr, sensorTypeBuffer);
		String type = String(sensorTypeBuffer);
		logger.print(tag, "\n\t sensorTypeBuffer =" + type);
		addr += res;		

		char sensorAddressBuffer[Sensor::sensorAddressLen];
		res = EEPROM_readAnything(addr, sensorAddressBuffer);
		logger.print(tag, "\n\t sensorAddressBuffer =" + String(sensorAddressBuffer));
		addr += res;
		String address = String(shield.sensorList.length() + 1);

		hiByte = EEPROM.read(addr++);
		lowByte = EEPROM.read(addr++);
		int sensorPin = word(hiByte, lowByte);
		logger.print(tag, "\n\t sensorPin=" + String(sensorPin));

		bool sensorEnabled = EEPROM.read(addr++);
		logger.print(tag, "\n\t sensorEnabled=" + String(sensorEnabled));
		//logger.print(tag, "\n");
		// init sensor
		Sensor* pSensor = nullptr;
		if (type == "onewiresensor") {
			//logger.print(tag, "\n\t onewiresensor found");
			pSensor = new OnewireSensor(sensorPin,sensorEnabled, address, name);

			OnewireSensor* pOnewireSensor = (OnewireSensor*)pSensor;
			hiByte = EEPROM.read(addr++);
			lowByte = EEPROM.read(addr++);
			int n = word(hiByte, lowByte);
			logger.print(tag, "\n\t temperatur sensor number=" + String(n));
			if (n < 0 || n > OnewireSensor::maxTempSensors) {
				n = 0;
			}
			for (int k = 0; k < n; k++) {

				hiByte = EEPROM.read(addr++);
				lowByte = EEPROM.read(addr++);
				int id = word(hiByte, lowByte);
				pOnewireSensor->temperatureSensors[k].id = id;
				//logger.print(tag, "\n\t onewire->temperatureSensors[k].id=" + String(pOnewireSensor->temperatureSensors[k].id));

				char sensorNameBuffer[Sensor::sensorNameLen];
				res = EEPROM_readAnything(addr, sensorNameBuffer);
				logger.print(tag, "\n\t sensorAddressBuffer =" + String(sensorNameBuffer));
				addr += res;
				pOnewireSensor->temperatureSensors[k].name = String(sensorNameBuffer);
			}
		}
		else if (type == "doorsensor") {
			//logger.print(tag, "doorsensor found");
			pSensor = new DoorSensor(sensorPin, sensorEnabled, address, name);
		}
		else if (type == "heatersensor") {
			//logger.print(tag, "doorsensor found");
			pSensor = new HeaterActuator(sensorPin, sensorEnabled, address, name);
		}
		else {
			logger.print(tag, "\n\t sensor type nor found");
			continue;
		}
	
		pSensor->init();
		shield.addSensor(pSensor);
	}

	shield.sensorList.show();

	logger.print(tag, "\n\n\t <<read Sensor Eprom ");
}

void initEPROM()
{
	logger.print(tag, "\n\n\t >>initEPROM");

	EEPROM.begin(512);

	byte id = EEPROM.read(ID_ADDR); // read the first byte from the EEPROM
	if (id == EEPROM_ID)
	{
		readEPROM();
	}
	else
	{
		writeEPROM();
	}
	logger.print(tag, "\n\n\t <<initEPROM");
}

void setupAP(void) {

	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	int n = WiFi.scanNetworks();
	logger.println(tag, "scan done");
	if (n == 0)
		logger.println(tag, "no networks found");
	else
	{
		logger.println(tag, n);
		logger.println(tag, " networks found");
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			logger.print(tag, i + 1);
			logger.print(tag, ": ");
			logger.print(tag, WiFi.SSID(i));
			logger.print(tag, " (");
			logger.print(tag, WiFi.RSSI(i));
			logger.print(tag, ")");
			logger.println(tag, (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
			delay(10);
		}
	}
	logger.print(tag, "\n");
	String st = "<ul>";
	for (int i = 0; i < n; ++i)
	{
		// Print SSID and RSSI for each network found
		st += "<li>";
		st += i + 1;
		st += ": ";
		st += WiFi.SSID(i);
		st += " (";
		st += WiFi.RSSI(i);
		st += ")";
		st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
		st += "</li>";
	}
	st += "</ul>";
	delay(100);
	String ssidName = String(ssidAP);
	for (int i = 0; i < sizeof(shield.MAC_array); ++i) {
		if (i > 0)
			ssidName += ":";
		ssidName += shield.MAC_array[i];
	}
	char buffer[100];
	ssidName.toCharArray(buffer, 100);
	logger.print(tag, "\nssidAP buffer= ");
	logger.println(tag, buffer);
	WiFi.softAP(buffer, passwordAP);
}

void checkOTA()
{
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	// ArduinoOTA.setHostname("myesp8266");

	// No authentication by default
	// ArduinoOTA.setPassword((const char *)"123");

	ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	//ArduinoOTA.handle();  uesta chiamata deve essere messa in loop()
}

char networkSSID[100/*Shield::networkSSIDLen*/];
char networkPassword[100/*Shield::networkPasswordLen*/];


bool testWifi() {
	Serial.begin(115200);
	delay(10);

	// We start by connecting to a WiFi network

	//Shield::getNetworkSSID().toCharArray(networkSSID,sizeof(networkSSID));
	//Shield::getNetworkPassword().toCharArray(networkPassword, sizeof(networkPassword));
	int i;
	for (i = 0; i < Shield::getNetworkSSID().length(); i++) {
		networkSSID[i] = Shield::getNetworkSSID().charAt(i);
	}
	networkSSID[i] = '\0';

	for (i = 0; i < Shield::getNetworkPassword().length(); i++) {
		networkPassword[i] = Shield::getNetworkPassword().charAt(i);
	}
	networkPassword[i] = '\0';

	
	logger.print(tag, "\n\t networkSSID=");
	logger.print(tag, networkSSID);
	logger.print(tag, "\n\t networkPassword=");
	logger.print(tag, networkPassword);


	logger.print(tag, "\n\t ssidtest=");
	logger.print(tag, networkSSID);
	logger.print(tag, "\n\t passwordtest=");
	logger.print(tag, passwordtest);


	Serial.println();
	Serial.println();
	Serial.print("Connecting");

	WiFi.mode(WIFI_STA);
	delay(2000);

	//char networkSSIDBuffer[Shield::networkSSIDLen];
	//Shield::getNetworkPassword().toCharArray(networkSSIDBuffer, sizeof(networkSSIDBuffer));
	//char networkPasswordBuffer[Shield::networkPasswordLen];
	//Shield::getNetworkPassword().toCharArray(networkPasswordBuffer, sizeof(networkPasswordBuffer));


	//WiFi.begin(networkSSID, networkPassword);
	WiFi.begin(ssidtest, passwordtest);
	//Serial.println(Shield::getNetworkSSID());

	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
	would try to act as both a client and an access-point and could cause
	network-issues with your other WiFi-devices on your WiFi-network. */
	WiFi.mode(WIFI_STA);
	WiFi.begin();

	int count = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
		if (count++ > 10) {
			Serial.println("WiFi connection timeout");
			return false;
		}
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	return true;
}

int testWifi2(void) {
	int c = 0;
	logger.println(tag, "Waiting for Wifi to connect");
	while (c < 15) {
		if (WiFi.status() == WL_CONNECTED) {
			logger.println(tag, "WiFi connected");
			return(20);
		}
		delay(500);
		logger.println(tag, WiFi.status());
		c++;
	}
	logger.println(tag, "Connect timed out, opening AP");
	return(10);
}

void setup()
{
	Serial.begin(115200);
	delay(10);

	Logger::init();
	logger.print(tag, "\n\t >>setup");
	logger.print(tag, "\n\n *******************RESTARTING************************");

	shield.init();
	shield.drawString(0, 0, "restarting..", 1, ST7735_WHITE);

	// Initialising the UI will init the display too.

	String str = "\n\nstarting.... Versione ";
	str += Shield::swVersion;
	logger.print(tag, str);

	// get MAC Address
	logger.print(tag, "\n\tMAC Address=");
	WiFi.macAddress(shield.MAC_array);
	for (int i = 0; i < sizeof(shield.MAC_array); ++i) {
		if (i > 0) sprintf(shield.MAC_char, "%s:", shield.MAC_char);
		sprintf(shield.MAC_char, "%s%02x", shield.MAC_char, shield.MAC_array[i]);

	}
	logger.print(tag, shield.MAC_char);


	shield.drawString(0, 20, "read eprom..", 1, ST7735_WHITE);
	initEPROM();
	

	//logger.print(tag, "\n\tSensorNames=" + sensorNames);
	shield.drawString(0, 80, "init onewire", 1, ST7735_WHITE);
	shield.drawString(0, 40, "connecting to WiFi", 1, ST7735_WHITE);
	// Connect to WiFi network
	logger.print(tag, "\n\nConnecting to " + Shield::getNetworkSSID() + " " + Shield::getNetworkPassword());

	
	if (testWifi()/* == 20*/) {

		checkOTA();

		shield.drawString(0, 50, "connected..Start server", 1, ST7735_WHITE);
		// Start the server
		server.begin();
		logger.print(tag, "Server started");
		shield.drawString(0, 60, "server started", 1, ST7735_WHITE);

		shield.localIP = WiFi.localIP().toString();
		// Print the IP address
		logger.println(tag, shield.localIP);

		wdt_disable();
		wdt_enable(WDTO_8S);

		readSensor();

		shield.drawString(0, 70, "init heater", 1, ST7735_WHITE);
		//shield.hearterActuator.init(String(shield.MAC_char));

		//shield.drawString(50, 70, String(shield.MAC_char), 1, ST7735_RED);
		//shield.drawString(60, 80, "DONE", 1, ST7735_WHITE);
		//shield.addActuators();

		Command command;
		shield.id = command.registerShield(shield);
		shield.drawString(0, 90, "registered" + String(shield.id), 1, ST7735_WHITE);

		if (shield.id != -1) {
			shieldRegistered = true;

			logger.print(tag, "\n\n\tSHIELD REGISTERED " + String(shield.id) + "\n");
		}
		else {
			shieldRegistered = false;
			logger.print(tag, "\n\n\tSHIELD NOT REGISTERED\n");
		}
	}
	else {
		logger.println(tag, "\n\n\tIMPOSSIBILE COLLEGARSI ALLA RETE\n");
		setupAP();

		server.begin();
		logger.print(tag, "\nLocal server started...192.168.4.1");
		shield.drawString(0, 60, "not connected. Local server 192.168.4.1", 1, ST7735_WHITE);
		// Print the IP address
		wdt_disable();
		wdt_enable(WDTO_8S);

		shield.phearterActuator->setStatus(Program::STATUS_DISABLED);
	}

	logger.print(tag, "\n\t lastRestartDate=" + Shield::getLastRestartDate());
	if (Shield::getLastRestartDate().equals("") == true) {
		Shield::setLastRestartDate(Logger::getStrDate());
		logger.print(tag, "\n\t *lastRestartDate=" + Shield::getLastRestartDate());
	}

	shield.drawString(0, 80, "restarted", 1, ST7735_WHITE);

	Command commannd;
	commannd.sendRestartNotification();

	shield.clearScreen();
	logger.print(tag, "\n\t <<setup");
}

String softwareReset(HttpRequest request, HttpResponse response) {

	logger.println(tag, F("\n\n\t >>software reset "));

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");
	data += F("SOFTWARE RESET - Attendere prego");
	data += F("</body></html>");

	response.send(HttpResponse::HTTPRESULT_OK, ESPWebServer::htmlContentType, data);

	delay(5000);

	ESP.restart();

	logger.println(tag, F("\n\t <<software reset "));
	return "";
}

String receiveCommand(HttpRequest request, HttpResponse response) {

	logger.println(tag, F("\n\n\t >>receiveCommand "));

	String str = request.body;
	String jsonResult = shield.sendCommand(str);

	logger.println(tag, "\n\t <<receiveCommand " + jsonResult);
	return jsonResult;
}

// imposta temperatura del sensore remoto. Chiamata periodicamente dal server
String setRemoteTemperature(HttpRequest request, HttpResponse response) {

	logger.print(tag, F("\n\n\t >>setRemoteTemperature: "));

	String str = request.body;
	POSTData data(str);

	if (data.has("temperature")) {
		String str = data.getString("temperature");
		logger.println(tag, "\n\t temperature=" + str);
		float temperature = str.toFloat();
		shield.phearterActuator->setRemote(temperature);
	}
	String result = "";
	result += getJsonStatus(request, response);

	logger.print(tag, "\n\t <<setRemoteTemperature: " + result);
	return result;
}

String showPower(HttpRequest request, HttpResponse response) {

	logger.print(tag, F("\n\n\t >>showPower "));

	logger.print(tag, F("\n\tprogramSettings.currentStatus="));
	logger.print(tag, shield.phearterActuator->getStatus());

	String str = request.body;
	POSTData posData(str);
	if (posData.has("status")) {
		String str = posData.getString("status");
		logger.println(tag, "\n\t status=" + str);
		if (str.equals("on") && shield.phearterActuator->getStatus() == Program::STATUS_DISABLED) {
			shield.phearterActuator->setStatus(Program::STATUS_IDLE);
			shield.phearterActuator->enableRele(true);

		}
		else if (str.equals("on")) {
			shield.phearterActuator->setStatus(Program::STATUS_DISABLED);
			shield.phearterActuator->enableRele(false);
		}
	}
	String data = "<result><rele>" + String(shield.phearterActuator->getReleStatus()) + "</rele>" +
		"<status>" + String(shield.phearterActuator->getStatus()) + "</status></result>";

	logger.print(tag, F("\n\t <<showPower "));
	return data;
}

// NON FUNZIONA PIU' DA RIPRISTINARE
String showwol(HttpRequest request, HttpResponse response) {

	logger.println(tag, "\n\t >>showwol ");

	/*wol* w = new wol();
	w->init();
	w->wakeup();
	delete w;*/

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>WOL</title></head><body>");
	data += F("</body></html>");
	//client.println(data);
	//client.stop();

	logger.println(tag, "\n\t >>showwol ");
	return data;
}


String getJsonStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\n\t >>getJsonStatus"));

	String data;
	data += "";
	data += shield.getSettingsJson();

	logger.print(tag, "\n\t <<getJsonStatus " + data + "\n");
	return data;
}
String getJsonSensorsStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\t >> getJsonSensorsStatus"));

	String data = "";
	data += shield.getSensorsStatusJson();

	logger.print(tag, "\n\t << getJsonSensorsStatus " + data + "\n");
	return data;
}

String getJsonActuatorsStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\t >> getJsonActuatorsStatus"));

	String data = "";
	//data += "HTTP/1.0 200 OK\r\nCont + ent-Type: application/json; ";
	//data += "\r\nPragma: no-cache\r\n\r\n";

	String json = shield.getActuatorsStatusJson();
	logger.println(tag, json);

	data += json;
	logger.print(tag, "\n\t << getJsonActuatorsStatus" + data + "\n");
	return data;
}

// DA ELIMINARE
String getJsonHeaterStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\t >> getJsonHeaterStatus"));

	String data = "";
	data += shield.getHeaterStatusJson();

	logger.print(tag, "\n\t <<getJsonActuatorsStatus" + data + "\n");
	return data;
}

// chiamata da jscript settings.html
String getJsonSettings(HttpRequest request, HttpResponse response) {
	logger.print(tag, F("\n\t >> getJsonSettings"));

	String data;
	data += "";
	data += shield.getSettingsJson();

	logger.print(tag, "\n\t << getJsonSettings" + data + "\n");
	return data;
}

void loop()
{
	ArduinoOTA.handle();  // questa chiamata deve essere messa in loop()

	wdt_enable(WDTO_8S);
	
	String page, param;
	client = server.available();
	HttpHelper http;

	ESPWebServer espWebServer(&client);
	if (client) {
		HttpRequest request = espWebServer.getHttpRequest();
		HttpRequest::HttpRequestMethod method = request.method;
		page = request.page;
		param = request.param;
		HttpResponse response = espWebServer.getHttpResponse();

		if (request.method != HttpRequest::HTTP_NULL) {

			logger.print(tag, "\n");
			logger.println(tag, ">>next HTTP request " + page);

			if (page.equalsIgnoreCase("command")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, receiveCommand(request, response));
			}
			else if (page.equalsIgnoreCase("temp")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, setRemoteTemperature(request, response));
			}
			else if (page.equalsIgnoreCase("power")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showPower(request, response));
			}
			else if (page.equalsIgnoreCase("wol")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showwol(request, response));
			}
			else if (page.equalsIgnoreCase("status")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonStatus(request, response));
			}
			else if (page.equalsIgnoreCase("settings")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonSettings(request, response));
			}
			else if (page.equalsIgnoreCase("heaterstatus")) { // DA ELIMINARE
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonHeaterStatus(request, response));
			}
			else if (page.equalsIgnoreCase("sensorstatus")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonSensorsStatus(request, response));
			}
			else if (page.equalsIgnoreCase("actuatorsstatus")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonActuatorsStatus(request, response));
			}
			else if (page.equalsIgnoreCase("reset")) {
				// la chiamara a  send è dentro software reset perchè il metodo non torna mai
				softwareReset(request, response);
			}
			else if (page.endsWith(".html") ||
				page.endsWith(".css") ||
				page.endsWith(".js") ||
				page.endsWith(".txt")) {
				response.sendVirtualFile(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, request.page);
			}
		}

		logger.println(tag, "<<next request " + page + "\n");
		logger.print(tag, "\n");

		return;
	}

	if (shield.id >= 0 && !WiFi.localIP().toString().equals(shield.localIP)) {

		Command command;
		logger.println(tag, "IP ADDRESS ERROR - re-register shield\n");
		command.registerShield(shield);
		shield.localIP = WiFi.localIP().toString();
		return;
	}

	shield.checkStatus();

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastFlash;
	if (timeDiff > flash_interval) {

		lastFlash = currMillis;
		if (shield.id <= 0) {
			Command command;
			logger.println(tag, F("ID NON VALIDO\n"));
			shield.id = command.registerShield(shield);
		}
		return;
	}

	if (currMillis - lastTimeSync > timeSync_interval) {
		Command command;
		lastTimeSync = currMillis;
		command.timeSync();
		return;
	}

	if (currMillis - lastSendLog > SendLog_interval) {
		lastSendLog = currMillis;
		//logger.send();
		return;
	}
}
