

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
#include "DS18S20Sensor.h"
#include <Time.h>
#include "TimeLib.h"
#include "POSTData.h"

//#include "Ucglib.h"

//#include "ESPDisplay.h"
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
int EPROM_Table_Schema_Version = 7;
//const char SWVERSION[] = "1.01";

Logger logger;
String sensorNames = "";

const char* ssidx = "Telecom-29545833"; // TP-LINK_3BD796
const char* password = "6oGzjkdMJU2q9XoQLfWiV3nj";
String tag = "Webduino";
const char *ssidAP = "ES8266";
const char *passwordAP = "thereisnospoon";
const char* ssid = "xxBUBBLES";
Shield shield;
//ESPDisplay display;

const byte EEPROM_ID = 0x99; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int TIMERTIME_ADDR = 1; // the EEPROM address used to store the pin
void initEPROM();
void readEPROM();
extern void writeEPROM();



// html page old style
String showMain(HttpRequest request, HttpResponse response);
String showHeater(HttpRequest request, HttpResponse response);
String showSettings(HttpRequest request, HttpResponse response);
String showIODevices(HttpRequest request, HttpResponse response);

// POST request
String receiveCommand(HttpRequest request, HttpResponse httpResponse);
String setRemoteTemperature(HttpRequest request, HttpResponse httpResponse);
String showPower(HttpRequest request, HttpResponse response);
String showRele(HttpRequest request, HttpResponse response);
String showChangeSettings(HttpRequest request, HttpResponse response);
String showChangeIODevices(HttpRequest request, HttpResponse response);
String softwareReset(HttpRequest request, HttpResponse response);

// GET request
String getJsonStatus(HttpRequest request, HttpResponse response);
String getJsonTempearatureSensorsStatus(HttpRequest request, HttpResponse response);
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

void writeEPROM() {

	logger.print(tag, "\n\n\t >>write EPROM");

	int addr = TIMERTIME_ADDR;
	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data
	byte hiByte;
	byte loByte;

	// dummy
	byte dummy;
	dummy = 3;
	EEPROM.write(addr++, dummy);
	logger.print(tag, "\n\t dummy = " + String(dummy));

	// build version
	hiByte = highByte(EPROM_Table_Schema_Version);
	loByte = lowByte(EPROM_Table_Schema_Version);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);

	// heater pin
	uint8_t pin = Shield::getHeaterPin();
	EEPROM.write(addr++, pin);
	logger.print(tag, "\n\t pin = " + String(pin));

	// heater enabled
	bool heaterEnabled = Shield::getHeaterEnabled();
	EEPROM.write(addr++, heaterEnabled);
	if (heaterEnabled)
		logger.print(tag, "\n\t heaterenabled = true");
	else
		logger.print(tag, "\n\t heaterenabled = false");

	// oneWire pin
	uint8_t oneWirePin = Shield::getOneWirePin();
	EEPROM.write(addr++, oneWirePin);
	logger.print(tag, "\n\t oneWirePin = " + String(oneWirePin));

	// onewire enabled
	bool temperatureSensorsEnabled = Shield::getTemperatureSensorsEnabled();
	EEPROM.write(addr++, temperatureSensorsEnabled);
	if (temperatureSensorsEnabled)
		logger.print(tag, "\n\t temperatureSensorsEnabled = true");
	else
		logger.print(tag, "\n\t temperatureSensorsEnabled = false");

	// io devices
	for (int i = 0; i < 10; i++) {
		hiByte = highByte(Shield::getIODevice(i));
		loByte = lowByte(Shield::getIODevice(i));
		EEPROM.write(addr++, hiByte);
		EEPROM.write(addr++, loByte);
		logger.print(tag, "\n\t iodevice = " + String(Shield::getIODevice(i)));
	}
	// local port
	int port = Shield::getLocalPort();
	hiByte = highByte(port);
	loByte = lowByte(port);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	logger.print(tag, "\n\t port = " + String(port));
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
	// board name
	char shieldNameBuffer[Shield::shieldNameLen];
	Shield::getServerName().toCharArray(shieldNameBuffer, sizeof(shieldNameBuffer));
	res = EEPROM_writeAnything(addr, shieldNameBuffer);
	addr += res;
	logger.print(tag, "\n\t shieldNameBuffer = " + String(shieldNameBuffer));
	// sensor names
	String str = "";
	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		str += String(sensor->sensorname);
		str += ";";
	}
	logger.print(tag, "\n\t str=" + str);
	char buffer[100];
	str.toCharArray(buffer, sizeof(buffer));
	res = EEPROM_writeAnything(addr, buffer);
	addr += res;
	EEPROM.commit();
	logger.println(tag, "\n\t <<write EPROM\n");
}

void readEPROM() {

	logger.print(tag, "\n\n\t >>read EPROM");

	byte hiByte;
	byte lowByte;
	int addr = TIMERTIME_ADDR;
	// dummy
	byte dummy = EEPROM.read(addr++);

	logger.print(tag, "\n\t dummy=" + String(dummy));

	logger.print(tag, "\EPROM_Table_Schema_Version=");
	logger.print(tag, String(EPROM_Table_Schema_Version));

	int epromversion = 0;
	// epromversion
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	epromversion = word(hiByte, lowByte);
	logger.print(tag, "\n\t epromversion=" + String(epromversion));

	if (epromversion >= 7/*EPROM_Table_Schema_Version*/) {
		uint8_t pin = EEPROM.read(addr++);
		Shield::setHeaterPin(pin);
		bool heaterEnabled = EEPROM.read(addr++);
		Shield::setHeaterEnabled(heaterEnabled);
		if (heaterEnabled)
			logger.println(tag, "\n\t heaterenabled = true");
		else
			logger.println(tag, "\n\t heaterenabled = false");

		uint8_t oneWirePin = EEPROM.read(addr++);
		Shield::setOneWirePin(oneWirePin);
		bool temperatureSensorsEnabled = EEPROM.read(addr++);
		Shield::setTemperatureSensorsEnabled(temperatureSensorsEnabled);
		if (temperatureSensorsEnabled)
			logger.println(tag, "\n\t temperatureSensorsEnabled = true");
		else
			logger.println(tag, "\n\t temperatureSensorsEnabled = false");
		
	}

	if (epromversion >= 7/*EPROM_Table_Schema_Version*/) {
		for (int i = 0; i < Shield::getMaxIoDevices(); i++) {
			hiByte = EEPROM.read(addr++);
			lowByte = EEPROM.read(addr++);
			Shield::setIODevice(i, word(hiByte, lowByte));

			logger.print(tag, "\ni=" + String(i));
			logger.print(tag, ", dev=" + String(Shield::getIODevice(i)));
		}
	}

	// local port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	int port = word(hiByte, lowByte);
	logger.print(tag, "\n\t port = ");
	logger.print(tag, String(port));
	Shield::setLocalPort(port);
	// ssid
	char networkSSIDBuffer[Shield::networkSSIDLen];
	int res = EEPROM_readAnything(addr, networkSSIDBuffer);
	//int res = EEPROM_readAnything(addr, buffer);
	logger.print(tag, "\n\t networkSSIDBuffer=" + String(networkSSIDBuffer));
	Shield::setNetworkSSID(String(networkSSIDBuffer)/*"TP-LINK_3BD796"*/);
	addr += res;
	// password
	char networkPasswordBuffer[Shield::networkPasswordLen];
	res = EEPROM_readAnything(addr, networkPasswordBuffer);
	//res = EEPROM_readAnything(addr, buffer);
	logger.print(tag, "\n\t networkPasswordBuffer = " + String(networkPasswordBuffer));
	Shield::setNetworkPassword(String(networkPasswordBuffer)/*"giacomocasa"*/);
	//Shield::setNetworkPassword("giacomocasa");
	addr += res;
	//server name
	char servernnameBuffer[Shield::serverNameLen];
	res = EEPROM_readAnything(addr, servernnameBuffer);
	logger.print(tag, "\n\tservernnameBuffer=" + String(servernnameBuffer));
	Shield::setServerName(String(servernnameBuffer));
	addr += res;
	// server port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	port = word(hiByte, lowByte);
	logger.print(tag, "\n\t port=" + String(port));
	Shield::setServerPort(port);
	// board name
	char shieldNameBuffer[Shield::shieldNameLen];
	res = EEPROM_readAnything(addr, shieldNameBuffer);
	logger.print(tag, "\n\t shieldNameBuffer =" + String(shieldNameBuffer));
	Shield::setShieldName(String(shieldNameBuffer));
	addr += res;
	// sensor names
	char buffer[100];
	res = EEPROM_readAnything(addr, buffer);
	addr += res;
	sensorNames = String(buffer);
	logger.print(tag, "\n\tsensorNames=" + sensorNames);
	/*char buffer[100];
	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		res = EEPROM_writeAnything(addr, buffer);
		sensor->sensorname = String(buffer);
		addr += res;
	}*/
	logger.print(tag, "\n\n\t <<read EPROM\n");
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

int testWifi(void) {
	int c = 0;
	logger.println(tag, "Waiting for Wifi to connect");
	while (c < 40) {
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

void setup()
{
	Serial.begin(115200);
	delay(10);
	
	Logger::init();
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


	//oneWirePtr = new OneWire(OneWirePin);
	//pDallasSensors = new DallasTemperature(oneWirePtr);
	shield.drawString(0, 20, "read eprom..", 1, ST7735_WHITE);
	initEPROM();
	logger.print(tag, "\n\tSensorNames=" + sensorNames);

	shield.drawString(0, 40, "connecting to WiFi", 1, ST7735_WHITE);
	// Connect to WiFi network
	logger.print(tag, "\n\nConnecting to " + Shield::getNetworkSSID() + " " + Shield::getNetworkPassword());

	WiFi.mode(WIFI_STA);//??????
	//delay(5000);

	char networkSSID[Shield::networkSSIDLen];
	Shield::getNetworkSSID().toCharArray(networkSSID, sizeof(networkSSID));
	char networkPassword[Shield::networkPasswordLen];
	Shield::getNetworkPassword().toCharArray(networkPassword, sizeof(networkPassword));
	logger.print(tag, "\n\tnetworkSSID=");
	logger.print(tag, networkSSID);
	logger.print(tag, "\n\tnetworkPassword=");
	logger.print(tag, networkPassword);

	WiFi.begin(networkSSID, networkPassword);
	if (testWifi() == 20/*WL_CONNECTED*/) {

		

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

		shield.drawString(0, 70, "init heater", 1, ST7735_WHITE);

		shield.hearterActuator.init(String(shield.MAC_char));

		shield.drawString(50, 70, String(shield.MAC_char), 1, ST7735_RED);
		
		shield.drawString(0, 80, "init onewire", 1, ST7735_WHITE);
		shield.addOneWireSensors(sensorNames);
		shield.drawString(60, 80, "DONE", 1, ST7735_WHITE);
		shield.addActuators();
		
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

		shield.hearterActuator.setStatus(Program::STATUS_DISABLED);
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

String setRemoteTemperature(HttpRequest request, HttpResponse response) {

	logger.print(tag, F("\n\n\t >>setRemoteTemperature: "));

	String str = request.body;
	POSTData data(str);

	if (data.has("temperature")) {
		String str = data.getString("temperature");
		logger.println(tag, "\n\t temperature=" + str);
		float temperature = str.toFloat();
		shield.hearterActuator.setRemote(temperature);
	}
	String result = "";
	result += getJsonStatus(request, response);

	logger.print(tag, "\n\t <<setRemoteTemperature: " + result);
	return result;
}

String showPower(HttpRequest request, HttpResponse response) {

	logger.print(tag, F("\n\n\t >>showPower "));

	logger.print(tag, F("\n\tprogramSettings.currentStatus="));
	logger.print(tag, shield.hearterActuator.getStatus());

	String str = request.body;
	POSTData posData(str);
	if (posData.has("status")) {
		String str = posData.getString("status");
		logger.println(tag, "\n\t status=" + str);
		if (str.equals("on") && shield.hearterActuator.getStatus() == Program::STATUS_DISABLED) {
			shield.hearterActuator.setStatus(Program::STATUS_IDLE);
			shield.hearterActuator.enableRele(true);

		}
		else if (str.equals("on")) {
			shield.hearterActuator.setStatus(Program::STATUS_DISABLED);
			shield.hearterActuator.enableRele(false);
		}
	}
	String data = "<result><rele>" + String(shield.hearterActuator.getReleStatus()) + "</rele>" +
		"<status>" + String(shield.hearterActuator.getStatus()) + "</status></result>";

	logger.print(tag, F("\n\t <<showPower "));
	return data;
}

String showRele(HttpRequest request, HttpResponse response) {

	logger.print(tag, F("\n\n\t >>called showRele "));

	String str = request.body;
	POSTData posData(str);
	logger.print(tag, "\n\t posData=" + posData.getDataString());

	if (posData.has("temperature")) {
		String str = posData.getString("temperature");
		shield.hearterActuator.setRemote(str.toFloat());
		logger.print(tag, "\n\t temperature=" + String(shield.hearterActuator.getRemoteTemperature()));
	}
	else {
		logger.print(tag, "\n\t invalid temperature");
	}
	// status
	int status;
	if (posData.has("status")) {
		String str = posData.getString("status");
		status = str.toInt();
		logger.print(tag, "\n\t status=" + status);
	}
	String command = "";
	switch (status) {
	case 0:
		command = "programoff";
		break;
	case 1:
		command = "programon";
		break;
	case 2:
		command = "disabled";
		break;
	case 3:
		command = "enabled";
		break;
	case 4:
		command = "manualoff";
		break;
	case 5:
		command = "manual";
		break;
	case 6:
		command = "manualend";
		break;
	}
	logger.print(tag, "\n\t command=" + command);
	// duration
	long duration = 0;
	if (posData.has("duration")) {
		duration = posData.getString("duration").toInt();
	}
	logger.print(tag, "\n\t duration=" + String(duration) + " minuti");
	duration = duration * 60 * 1000;
	logger.print(tag, "\n\t duration =" + String(duration) + " millisecondi");
	// sensorid
	int sensorId = 0;
	if (posData.has("sensorid")) {
		sensorId = posData.getString("sensorid").toInt();
	}
	logger.print(tag, "\n\t sensorId=" + sensorId);
	// target
	float target = 0;
	if (posData.has("target")) {
		target = posData.getString("target").toFloat();
	}
	logger.print(tag, "\n\t target =" + String(target));
	shield.hearterActuator.setTargetTemperature(target);
	// remote temperature
	float remoteTemperature = -1;
	if (posData.has("temperature")) {
		remoteTemperature = posData.getString("temperature").toFloat();
		logger.print(tag, "\n\t remoteTemperature =" + String(remoteTemperature));
		shield.hearterActuator.setRemote(remoteTemperature);
	}
	else {
		logger.print(tag, F("\n\t remoteTemperature MISSING"));
	}
	// program
	int program = 0;
	if (posData.has("program")) {
		program = posData.getString("program").toInt();
	}
	logger.print(tag, "\n\t program=" + program);
	// timerange
	int timerange = 0;
	if (posData.has("timerange")) {
		timerange = posData.getString("timerange").toInt();
	}
	logger.print(tag, "\n\t timerange=" + timerange);
	// local sensor
	int localsensor = 0;
	if (posData.has("localsensor")) {
		localsensor = posData.getString("localsensor").toInt();
	}
	logger.print(tag, "\n\t localsensor=" + localsensor);

	shield.hearterActuator.changeProgram(command, duration,
		!localsensor,
		remoteTemperature,
		sensorId,
		target, program, timerange);

	String data;
	data += "";
	data += F("<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");
	data += F("</body></html>");

	logger.print(tag, F("\n\t <<end show rele\n"));
	return data;
}

String showChangeSettings(HttpRequest request, HttpResponse response) {

	logger.println(tag, F("\n\t >>showChangeSettings "));

	String str = request.body;
	POSTData posData(str);
	logger.print(tag, "\n\t posData=" + posData.getDataString());

	// localport
	if (posData.has("localport")) {
		int localport = posData.getString("localport").toInt();
		Shield::setLocalPort(localport);
		logger.print(tag, "\n\t localPort=" + Shield::getLocalPort());
	}
	// ssid
	if (posData.has("ssid")) {
		Shield::setNetworkSSID(posData.getString("ssid"));
		logger.print(tag, "\n\t networkSSID=" + Shield::getNetworkSSID());
	}
	// password
	if (posData.has("password")) {
		Shield::setNetworkPassword(posData.getString("password"));
		logger.print(tag, "\n\t networkPassword=" + Shield::getNetworkPassword());
	}
	// server name
	if (posData.has("servername")) {
		Shield::setServerName(posData.getString("servername"));
		logger.print(tag, "\n\t servername=" + Shield::getServerName());
	}
	// server port
	if (posData.has("serverport")) {
		Shield::setServerPort(posData.getString("serverport").toInt());
		logger.print(tag, "\n\tserver port=" + Shield::getServerPort());
	}
	// shield name
	if (posData.has("shieldname")) {
		Shield::setShieldName(posData.getString("shieldname"));
		logger.print(tag, "\n\shieldName=" + Shield::getShieldName());
	}
	// sensor names
	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor*  sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		char buffer[20];
		String str = "sensor" + String(i + 1);
		str.toCharArray(buffer, sizeof(buffer));
		logger.print(tag, "\n\tbuffer=");
		logger.print(tag, buffer);
		if (posData.has(buffer)) {
			sensor->sensorname = posData.getString(buffer);
			logger.print(tag, "\n\t " + String(buffer) + "=" + sensor->sensorname);
			logger.print(tag, "\n\t sensor->sensorname=" + sensor->sensorname);
		}
	}
	String data = "";
	data += F("<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main'><title>Timer</title></head><body></body></html>");
	data += F("</body></html>");

	writeEPROM();

	logger.println(tag, F("\n\t >>showChangeSettings "));
	return data;
}

String showChangeIODevices(HttpRequest request, HttpResponse response) {

	logger.println(tag, "\n\t >>showChangeIODevices ");

	String str = request.body;
	POSTData posData(str);
	logger.print(tag, "\n\t posData=" + posData.getDataString());

	for (int i = 0; i < Shield::getMaxIoDevices(); i++) {

		char buffer[20];
		String str = "iodevice" + String(i + 1);
		str.toCharArray(buffer, sizeof(buffer));

		if (posData.has(buffer)) {
			Shield::setIODevice(i, posData.getString(buffer).toInt());
			logger.print(tag, "\n\t" + String(buffer) + "=" + String(Shield::getIODevice(i)));
		}
	}
	String data;
	data += "";
	data += F("<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main'><title>Timer</title></head><body></body></html>");
	data += F("</body></html>");
	writeEPROM();
	logger.println(tag, "\n\t <<showChangeIODevices ");
}

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

String getJsonTempearatureSensorsStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\t >> getJsonTempearatureSensorsStatus"));

	String data = "";
	data += shield.getSensorsStatusJson();


	logger.print(tag, "\n\t << getJsonTempearatureSensorsStatus " + data + "\n");
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

String getJsonHeaterStatus(HttpRequest request, HttpResponse response)
{
	logger.print(tag, F("\n\t >> getJsonHeaterStatus"));

	String data = "";
	data += shield.getHeaterStatusJson();

	logger.print(tag, "\n\t <<getJsonActuatorsStatus" + data + "\n");
	return data;
}

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

	//////////////////
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
			logger.println(tag, ">>next request " + page);

			if (page.equalsIgnoreCase("main")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showMain(request, response));
			}
			else if (page.equalsIgnoreCase("heater")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showHeater(request, response));
			}
			else if (page.equalsIgnoreCase("setting")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showSettings(request, response));
			}
			else if (page.equalsIgnoreCase("iodevices")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showIODevices(request, response));
			}


			else if (page.equalsIgnoreCase("command")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, receiveCommand(request, response));
			}
			else if (page.equalsIgnoreCase("temp")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, setRemoteTemperature(request, response));
			}
			else if (page.equalsIgnoreCase("power")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showPower(request, response));
			}
			else if (page.equalsIgnoreCase("rele")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showRele(request, response));
			}
			else if (page.equalsIgnoreCase("chstt")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showChangeSettings(request, response));
			}
			else if (page.equalsIgnoreCase("chiodevices")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::htmlContentType, showChangeIODevices(request, response));
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
			else if (page.equalsIgnoreCase("heaterstatus")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonHeaterStatus(request, response));
			}
			else if (page.equalsIgnoreCase("temperaturesensorstatus")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonTempearatureSensorsStatus(request, response));
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




String showMain(HttpRequest request, HttpResponse response)
{
	logger.println(tag, "\n\t >>showMain ");

	String data;
	data += "";
	data += F("<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");
	// comandi
	data += F("\n<font color='#53669E' face='Verdana' size='2'><b>Webduino - ");
	data += Logger::getStrDate();
	data += F("</b></font>");
	data += F("\r\n<table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// power
	data += F("<tr><td>Power ");
	if (shield.hearterActuator.getStatus() == Program::STATUS_DISABLED) {
		data += F("OFF</td><td><form action='/power' method='POST'>");
		data += F("<input type='hidden' name='status' value='on' >");
		data += F("<input type='submit' value='on'></form>");
	}
	else {
		data += F("ON</td><td><form action='/power' method='POST'>");
		data += F("<input type='hidden' name='status' value='off' >");
		data += F("<input type='submit' value='off'></form>");
	}
	data += F("</td></tr>");
	// status & rele	
	data += "<tr><td>Actuator:</td><td>";
	data += shield.hearterActuator.getStatusName();
	data += " - Rele: ";
	if (shield.hearterActuator.getReleStatus()) {
		data += F("on - ");
	}
	else {
		data += F("off - ");
	}
	if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE ||
		shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO ||
		shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		time_t onStatusDuration = (millis() - shield.hearterActuator.programStartTime) / 1000;
		time_t duration = (shield.hearterActuator.programDuration) / 1000;
		time_t remainingTime = (duration - onStatusDuration);
		time_t hr = remainingTime / 3600L;
		time_t mn = (remainingTime - hr * 3600L) / 60L;
		time_t sec = (remainingTime - hr * 3600L) % 60L;

		data += F(" onStatusDuration:");
		data += String(onStatusDuration);
		data += F(" ");
		data += String(onStatusDuration / 60);
		data += F(" duration:");
		data += String(duration);
		data += F(" ");
		data += String(duration / 60L);
		data += F(" remainingTime:");
		data += String(remainingTime);
		data += F(" ");
		data += String(remainingTime / 60L);
	}
	data += F("</td></tr>");
	// program
	data += F("<tr><td>program </td><td>");
	if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE) {
		data += F("program ");
		data += shield.hearterActuator.getActiveProgram();
		data += F("timerange ");
		data += shield.hearterActuator.getActiveTimeRange();

		data += " Target: " + String(shield.hearterActuator.getTargetTemperature()) + "°C";
		data += F(" Sensor: ");
		if (shield.hearterActuator.sensorIsRemote()) {
			data += " Remote (" + String(shield.hearterActuator.getRemoteSensorId()) + ")";
			data += String(shield.hearterActuator.getRemoteTemperature()) + "°C";
		}
		else {
			data += " Local (" + String(shield.hearterActuator.getLocalSensorId()) + ")";
			data += String(shield.hearterActuator.getLocalTemperature()) + "°C";
		}
	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO) {

		data += F("program manual auto");

	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F("program manual  off");
	}
	data += F("</td></tr>");
	// Manual
	data += F("<tr><td>Manual-- ");
	if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO || shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F("<tr><td>Manual ON</td><td>");

		if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO) {
			char buffer[200];
			sprintf(buffer, "Target: %d.%02d<BR>", (int)shield.hearterActuator.getTargetTemperature(), (int)(shield.hearterActuator.getTargetTemperature() * 100.0) % 100);
			data += String(buffer);

			if (!shield.hearterActuator.sensorIsRemote()) {
				data += F("Sensor: Local");
			}
			else {
				data += "Sensor: Remote (" + String(shield.hearterActuator.getRemoteSensorId()) + ")";
			}
		}
		data += F("<form action='/rele' method='POST'>");
		//data += F("<input type='hidden' name='manual' value='3'>"); 
		data += F("<input type='hidden' name='status' value='6'>"); // 6 = manual end  
		data += F("<input type='submit' value='stop manual'></form>");
	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE || shield.hearterActuator.getStatus() == Program::STATUS_IDLE) {
		data += F("OFF</td><td><form action='/rele' method='POST'>");
		data += F("Minutes:<input type='num' name='duration' value='");
		data += 30;
		data += F("' size='5' maxlength='5'><BR>");
		data += F("<input type='hidden' name='status' value='5'>"); // 5 = manual auto
		data += F("Target:<input type='number' name='target' value='22.0' step='0.10' ><BR>");
		data += F("Sensor:<input type='radio' name='sensor' value='0' checked>Local<input type='radio' name='sensor' value='1'>Remote<BR>");
		data += F("<input type='submit' value='start manual'></form>");

		// manual off
		data += F("<form action='/rele' method='POST'>");
		data += F("Minutes:<input type='num' name='duration' value='");
		data += 30;
		data += F("' size='5' maxlength='5'><BR>");
		data += F("<input type='hidden' name='status' value='4'>"); // 4 = manual off 
		data += F("<input type='submit' value='start manual off'></form>");
	}
	data += F("</td></tr>");
	// temperature sensor
	int count = 0;
	DS18S20Sensor* pSensor = (DS18S20Sensor*)shield.sensorList.getFirst();
	while (pSensor != NULL) {

		logger.print(tag, "\n\tpSensor->sensorname=" + pSensor->sensorname);

		count++;
		data += "<tr><td>Sensor " + String(count) + " [" + pSensor->getSensorAddress() + "]</td>"
			+ "<td>" + String(pSensor->getTemperature()) + "°C (Av."
			+ String(pSensor->getAvTemperature()) + "°C)"
			+ "<br><form action='/chstt' method='POST'>"
			+ "<input type = 'num' name = 'sensor" + String(count) + "' value='" + pSensor->sensorname + "' "
			+ "size='32' maxlength='32'>"
			+ "<input type='submit' value='save'/>"
			+ "</form></td></tr>";
		pSensor = (DS18S20Sensor*)shield.sensorList.getNext();
	}
	// sofware reset
	data += F("<tr><td>Software reset</td><td><form action='/reset' method='POST'><input type='submit' value='reset'></form></td></tr>");

	// MACAddress
	data += "<tr><td>MACAddress: </td><td>";
	data += String(shield.MAC_char);
	data += "</td></tr>";

	// localip
	String ip = WiFi.localIP().toString();
	char ip1[20], ip2[20];
	ip.toCharArray(ip1, sizeof(ip1));
	shield.localIP.toCharArray(ip2, sizeof(ip2));
	data += "<tr><td>Local IP: </td><td>" + WiFi.localIP().toString() +
		"(" + shield.localIP + ")</td></tr>";
	// shield id
	data += "<tr><td>Shield Id</td><td>" + String(shield.id) + "<form action='/register' method='POST'><input type='submit' value='register'></form></td></tr>";

	data += F("</table>");
	data += Shield::swVersion;
	data += "\nEPROM_Table_Schema_Version=" + String(EPROM_Table_Schema_Version);
	data += F("</body></html>");

	logger.println(tag, "\n\t <<showMain ");
	return data;
}

String showHeater(HttpRequest request, HttpResponse response)
{
	logger.println(tag, F("\n\t >>showHeater "));

	String data;
	data += "";
	data += F("<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");
	data += F("\n<font color='#53669E' face='Verdana' size='2'><b>Webduino - ");
	data += Logger::getStrDate();
	data += F("</b></font>");
	data += F("\r\n<table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// status & rele	
	data += "<tr><td>Actuator:</td><td>";
	data += shield.hearterActuator.getStatusName();
	data += " - Rele: ";
	if (shield.hearterActuator.getReleStatus()) {
		data += F("on - ");
	}
	else {
		data += F("off - ");
	}

	if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE ||
		shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO ||
		shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		time_t onStatusDuration = (millis() - shield.hearterActuator.programStartTime) / 1000;
		time_t duration = (shield.hearterActuator.programDuration) / 1000;
		time_t remainingTime = (duration - onStatusDuration);

		time_t hr = remainingTime / 3600L;
		time_t mn = (remainingTime - hr * 3600L) / 60L;
		time_t sec = (remainingTime - hr * 3600L) % 60L;

		data += F(" onStatusDuration:");
		data += String(onStatusDuration);
		data += F(" ");
		data += String(onStatusDuration / 60);
		data += F(" duration:");
		data += String(duration);
		data += F(" ");
		data += String(duration / 60L);
		data += F(" remainingTime:");
		data += String(remainingTime);
		data += F(" ");
		data += String(remainingTime / 60L);
	}
	data += F("</td></tr>");

	// program
	data += F("<tr><td>program </td><td>");
	if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE) {
		data += F("program ");
		data += shield.hearterActuator.getActiveProgram();
		data += F("timerange ");
		data += shield.hearterActuator.getActiveTimeRange();

		data += " Target: " + String(shield.hearterActuator.getTargetTemperature()) + "°C";
		data += F(" Sensor: ");
		if (shield.hearterActuator.sensorIsRemote()) {
			data += " Remote (" + String(shield.hearterActuator.getRemoteSensorId()) + ")";
			data += String(shield.hearterActuator.getRemoteTemperature()) + "°C";
		}
		else {
			data += " Local (" + String(shield.hearterActuator.getLocalSensorId()) + ")";
			data += String(shield.hearterActuator.getLocalTemperature()) + "°C";
		}
	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO) {

		data += F("program manual auto");
	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F("program manual  off");
	}
	data += F("</td></tr>");
	// Manual
	data += F("<tr><td>Manual-- ");
	if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO || shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F("<tr><td>Manual ON</td><td>");

		if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO) {
			char buffer[200];
			sprintf(buffer, "Target: %d.%02d<BR>", (int)shield.hearterActuator.getTargetTemperature(), (int)(shield.hearterActuator.getTargetTemperature() * 100.0) % 100);
			data += String(buffer);

			if (!shield.hearterActuator.sensorIsRemote()) {
				data += F("Sensor: Local");
			}
			else {
				data += "Sensor: Remote (" + String(shield.hearterActuator.getRemoteSensorId()) + ")";
			}
		}
		data += F("<form action='/rele' method='POST' id='manualForm'>");
		//data += F("<input type='hidden' name='manual' value='3'>"); 
		data += F("<input type='hidden' name='status' value='6'>"); // 6 = manual end  
		data += F("<input type='submit' value='stop manual'></form>");
	}
	else if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE || shield.hearterActuator.getStatus() == Program::STATUS_IDLE) {
		data += F("OFF</td><td><form action='/rele' method='POST'  id='manualForm'>");
		data += F("Minutes:<input type='num' name='duration' value='");
		data += 30;
		data += F("' size='5' maxlength='5'><BR>");
		data += F("<input type='hidden' name='status' value='5'>"); // 5 = manual auto
		data += F("Target:<input type='number' name='target' value='22.0' step='0.10' ><BR>");
		data += F("Sensor:<input type='radio' name='sensor' value='0' checked>Local<input type='radio' name='sensor' value='1'>Remote<BR>");
		data += F("<input type='submit' value='start manual'></form>");

		// manual off
		data += F("<form action='/rele' method='POST' id='manualForm'>");
		data += F("Minutes:<input type='num' name='duration' value='");
		data += 30;
		data += F("' size='5' maxlength='5'><BR>");
		data += F("<input type='hidden' name='status' value='4'>"); // 4 = manual off 
		data += F("<input type='submit' value='start manual off'></form>");
	}
	data += F("</td></tr>");

	data += F("</table>");
	data += Shield::swVersion;
	data += "\nEPROM_Table_Schema_Version=" + String(EPROM_Table_Schema_Version);

	data += "<script>"
		"var form = document.getElementById('manualForm');"
		"form.onsubmit = function(e) {"
		"e.preventDefault();"
		"var data = {};"
		"for (var i = 0, ii = form.length; i < ii; ++i) {"
		"var input = form[i];"
		"if (input.name) {"
		"data[input.name] = input.value;"
		"}"
		"}"

		"alert('The form was submitted'+JSON.stringify(data));"

		"var xhr = new XMLHttpRequest();"
		"xhr.open(form.method, form.action, true);"
		"xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');"
		"xhr.send(JSON.stringify(data));"
		"xhr.onloadend = function() {"
		"};"
		"};"
		""

		"function sendCommand() {"
		"alert('The form was submitted');"
		"}"
		"</script>";

	data += F("</body></html>");

	logger.println(tag, F("\n\t <<showHeater "));
	return data;
}

String showSettings(HttpRequest request, HttpResponse response)
{
	logger.println(tag, F("\n\t >>showSettings "));

	String data;
	data += "";
	data += F("<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");

	data += F("<font color='#53669E' face='Verdana' size='2'><b>Impostazioni </b></font>\r\n<form action='/chstt' method='POST'><table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// local port
	data += F("<tr><td>Local port</td><td><input type='num' name='localport");
	data += F("' value='");
	data += Shield::getLocalPort();
	data += F("' size='4' maxlength='4'> </td></tr>");
	// board name
	data += F("<tr><td>Board name</td><td><input type='num' name='shieldname' value='");
	data += String(Shield::getShieldName());
	data += F("' size='");
	data += String(shield.shieldNameLen - 1);
	data += F("' maxlength='");
	data += String(shield.shieldNameLen - 1);
	data += F("'> </td></tr>");
	// ssid
	data += F("<tr><td>SSID</td><td><input type='num' name='ssid");
	data += F("' value='");
	data += String(Shield::getNetworkSSID());
	data += F("' size='32' maxlength='32'> </td></tr>");
	// password
	data += F("<tr><td>password</td><td><input type='num' name='password");
	data += F("' value='");
	data += String(Shield::getNetworkPassword());
	data += F("' size='96' maxlength='96'> </td></tr>");
	// server name
	data += F("<tr><td>Server name</td><td><input type='num' name='servername' value='");
	data += String(Shield::getServerName());
	data += F("' size='");
	data += (Shield::serverNameLen - 1);
	data += F("' maxlength='");
	data += (Shield::serverNameLen - 1);
	data += F("'> </td></tr>");
	// server port
	data += F("<tr><td>Server port</td><td><input type='num' name='serverport");
	data += F("' value='");
	data += String(Shield::getServerPort());
	data += F("' size='4' maxlength='4'> </td></tr>");

	data += F("</table><input type='submit' value='save'/></form>");
	data += F("</body></html>");

	logger.println(tag, F("\n\t >>showSettings "));
	return data;

}

String showIODevices(HttpRequest request, HttpResponse response)
{
	logger.println(tag, F("\n\t >>showIODevices "));

	String data = "";
	data += F("<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");
	data += F("<font color='#53669E' face='Verdana' size='2'><b>IO devices </b></font>\r\n<form action='/chiodevices' method='POST'><table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");

	for (int i = 0; i < Shield::getMaxIoDevices(); i++) {
		// local port
		data += F("<tr><td>device ");
		data += String(i);
		data += F("</td><td>");

		data += F("<select name='iodevice");
		data += String(i);
		data += F("' >");

		for (int k = 0; k < Shield::getMaxIoDeviceTypes(); k++) {

			data += F("<option value='");
			data += String(k);
			if (Shield::getIODevice(i) == k)
				data += F(" selected ");
			data += F("' >");
			data += Shield::getIoDevicesTypeName(k);
			data += F("</option>");
		}
		data += F("</select>");
		data += F("</td></tr>");
	}
	data += F("</table><input type='submit' value='save'/></form>");
	data += F("</body></html>");

	logger.println(tag, F("\n\t >>showIODevices "));
	return data;
}


