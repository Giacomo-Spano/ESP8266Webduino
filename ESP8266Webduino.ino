

#include <PubSubClient.h>
#include "stdafx.h"

#include "ObjectClass.h"
#include "OnewireSensor.h"
#include "JSONArray.h"
#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "DoorSensor.h"

#include "TFTDisplay.h"
#include "HttpResponse.h"
#include "ESPWebServer.h"
#include "HttpRequest.h"
#include "ESP8266Webduino.h"
#include <ESP8266WiFi.h>

#include "MQTTClientClass.h"
#include "MyEPROMClass.h"

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

#include "JSONObject.h"

// HTTP Update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
bool checkHTTPUpdate = true; //true;
//

#define production "1"
//#define wifibox "1"


extern "C" {
#include "user_interface.h"
}
//uint32_t free = system_get_free_heap_size();



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

const int maxjsonLength = 1000;

String tag = "Webduino";
const char *ssidAP = "ES8266";
const char *passwordAP = "thereisnospoon";
static Shield shield;

String siid = "TP-LINK_3BD796";
String pass = "giacomocasa";
//siid = "TP-LINK_3D88";
//pass = "giacomobox";

// EPROM
const byte EEPROM_ID = 0x96; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int SWVERSION_ADDR = 1;
const int CREDENTIAL_ADDR = SWVERSION_ADDR + 10; // the EEPROM address used to store the pin
const int SETTINGS_ADDR = CREDENTIAL_ADDR + Shield::networkSSIDLen + Shield::networkPasswordLen + 2; // the EEPROM address used to store the pin
const int SENSOR_ADDR = SETTINGS_ADDR + 1000; // the EEPROM address used to store the pin

int epromversion = 0;

void initEPROM();
void readEPROM();
extern void writeEPROM();
extern void resetEPROM();
extern void writeSettings();
extern void readSettings(JSONObject *json);

bool updateTime();

// POST request
String receiveCommand(HttpRequest request, HttpResponse httpResponse);
String setRemoteTemperature(HttpRequest request, HttpResponse httpResponse);
String showPower(HttpRequest request, HttpResponse response);
String softwareReset(HttpRequest request, HttpResponse response);

// GET request
String getJsonStatus(HttpRequest request, HttpResponse response);
String getJsonSettings(HttpRequest request, HttpResponse response);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;

extern bool mqtt_publish(String topic, String message);
MQTTClientClass mqttclient;
long lastMsg = 0;
int messagenumber = 0;
//String topic = "sendtopic";
static const char googleServerName[] = "google.com";
static const char giacomohomeServerName[] = "giacomohome.ddns.net";
static const char giacomoboxServerName[] = "giacomobox.ddns.net";

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
	// scrive zero nel primo settore della EPROM
	// così al riavvio successivo saranno ripristinati i valori di default
	logger.print(tag, "\n\n\t >>resetEPROM");

	byte hiByte;
	byte loByte;
	EEPROM.write(ID_ADDR, 0/*EEPROM_ID*/); // delete EEPROM_ID to indicate invalid data

	EEPROM.commit();
	logger.println(tag, "\n\t <<resetEPROM");
}

void writeSettings() {

	logger.print(tag, "\n");
	logger.println(tag, "\n\t >>writeSettings");

	JSONObject json;
			
	// local port
	int port = Shield::getLocalPort();
	logger.print(tag, "\n\t port = " + String(port));
	json.pushInteger("port", port);

	// server name
	String serverName = Shield::getServerName();
	logger.print(tag, "\n\t serverName = " + serverName);
	json.pushString("servername", serverName);

	// server port
	int serverport = Shield::getServerPort();
	logger.print(tag, "\n\t serverport = " + String(serverport));
	json.pushInteger("serverport", serverport);

	// shieldName name
	String shieldname = Shield::getShieldName();
	logger.print(tag, "\n\t shieldname = " + shieldname);
	json.pushString("shieldname", shieldname);

	MyEPROMClass eprom;
	logger.print(tag, "\n" + logger.formattedJson(json.toString()));
	eprom.writeJSON(SETTINGS_ADDR, &json);
	
	logger.println(tag, "<<writeSettings\n");
}

String getSettingsFromServer() {

	//logger.print(tag, "\n");
	logger.println(tag, ">>getSettingsFromServer");

	Command command;
	String settings = command.loadShieldSettings();	

	logger.println(tag, "<<getSettingsFromServer settings=" + settings);
	return settings;
}

void readSettings(JSONObject *json) {

	logger.println(tag, "\n\t >>readSettings");
	
	// parse json
	// siid
	/*siid = "";
	if (json->has("siid"))
		siid = json->getString("siid");
	siid += '\0';
	logger.print(tag, "\n\t siid=" + siid);
	// password
	pass = "";
	if (json->has("passwd"))
		pass = json->getString("passwd");
	pass += '\0';
	logger.print(tag, "\n\t pass=" + pass);*/

	// port
	int port = 0;
	if (json->has("port"))
		port = json->getInteger("port");
	logger.print(tag, "\n\t port=" + String(port));
	Shield::setLocalPort(port);
	// server name
	String servername = "";
	if (json->has("servername"))
		servername = json->getString("servername");
	logger.print(tag, "\n\t servername=" + servername);
	Shield::setServerName(servername);	
	// serverport
	int serverport = 0;
	if (json->has("serverport"))
		serverport = json->getInteger("serverport");
	logger.print(tag, "\n\t serverport=" + String(serverport));
	Shield::setServerPort(serverport);

	// mqtt server
	String mqttserver = "";
	if (json->has("mqttserver"))
		mqttserver = json->getString("mqttserver");
	logger.print(tag, "\n\t mqttserver=" + servername);
	Shield::setMQTTServer(servername);
	// mqtt port
	int mqttport = 0;
	if (json->has("mqttport"))
		mqttport = json->getInteger("mqttport");
	logger.print(tag, "\n\t mqttport=" + String(mqttport));
	Shield::setMQTTPort(mqttport);


	// shield name
	String shieldname = "";
	if (json->has("shieldname"))
		shieldname = json->getString("shieldname");
	logger.print(tag, "\n\t shieldname=" + shieldname);
	Shield::setShieldName(shieldname);	

	logger.print(tag, "\n\t <<readSettings");
}

void writeEPROM() {

	logger.print(tag, "\n\n\t >>write EPROM");

	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data

	//int index = 1;
	// build version
	MyEPROMClass eprom;
	eprom.writeInt(SWVERSION_ADDR, EPROM_Table_Schema_Version);

	writeSettings();
	
	writeSensors();

	//siid = Shield::getNetworkSSID();
	//pass = Shield::getNetworkPassword();

	saveCredentials(CREDENTIAL_ADDR);

	logger.print(tag, "\n\t <<write EPROM\n");
}

void writeSensors() {

	logger.print(tag, "\n");
	logger.println(tag, ">>writeSensors");

	MyEPROMClass eprom;
	// sensor number
	int index = SENSOR_ADDR;
	index += eprom.writeInt(index, shield.sensorList.length());
		
	for (int i = 0; i < shield.sensorList.length(); i++) {
		logger.print(tag, "\n\n\t sensor#: " + String(i));
		Sensor* sensor = (Sensor*)shield.sensorList.get(i);
		JSONObject json2;
		sensor->getJSON(&json2);
		logger.print(tag, "\n" + logger.formattedJson(json2.toString()));

		index += eprom.writeJSON(index, &json2);
	}

	logger.println(tag, "<<writeSensors\n");
}


void readEPROM() {

	logger.println(tag, "\t >>read EPROM");
	
	// check EPROM_ID
	byte epromID = EEPROM.read(ID_ADDR);
	logger.print(tag, "\n\t epromID=" + String(epromID));

	if (epromID != EEPROM_ID) { // reset EPROM_ID
		logger.print(tag, "\n\t INVALID DATA" + String(epromID));
		writeEPROM();
		return;
	}
	MyEPROMClass eprom;
	// epromversion
	eprom.readInt(SWVERSION_ADDR,&epromversion);
	logger.print(tag, "\n\t epromversion=" + String(epromversion));
	
	// load netword credential
	loadCredentials(CREDENTIAL_ADDR);
	
	// load settings
	JSONObject json;	
	eprom.readJSON(SETTINGS_ADDR, &json);
	logger.print(tag, "\n\n\t json:" + logger.formattedJson(json.toString()));
	readSettings(&json);
	
	logger.print(tag, "\n\n\t <<read EPROM\n");
}

// carica i settings dal server
bool loadSensors(String settings) {

	logger.print(tag, "\n\n");
	logger.println(tag, ">>loadSensors settings=" + logger.formattedJson(settings));
	
	JSONObject json(settings);	

	if (json.has("shieldid")) {
		int id = json.getInteger("shieldid");
		Shield::setShieldId(id);

		if (json.has("name")) {
			String name = json.getString("name");
			Shield::setShieldName(name);
		}
	}
	else {
		logger.println(tag, "error -ID MISSING");
		return false;
	}

	if (json.has("sensors")) {
		String str = json.getJSONArray("sensors");
		JSONArray sensors(str);

		String sensorstr = sensors.getFirst();
		while (sensorstr != "") {


			logger.print(tag, "\n\n\t SENSOR: " + sensorstr);
			JSONObject jObject(sensorstr);
			logger.print(tag, "\n: " + logger.formattedJson(jObject.toString()));

			Sensor* sensor = SensorFactory::createSensor(&jObject);
			
			if (sensor != nullptr) {
				logger.print(tag, "\n\n\t sensor=" + sensor->toString());
				shield.sensorList.add(sensor);
			}

			sensorstr = sensors.getNext();
		}

	}

	return true;
	logger.println(tag, "<<loadSensors\n");
}

void readSensors() {

	logger.println(tag, ">>read Sensors");

	int index = SENSOR_ADDR;
	MyEPROMClass eprom;
	int sensorCount;
	index += eprom.readInt(index, &sensorCount);
	logger.print(tag, "\n\t sensorCount=" + String(sensorCount));
	if (sensorCount < 0 || sensorCount > Shield::maxSensorNum) {
		logger.print(tag, "\n\t INVALID VALUE!");
		sensorCount = 0;
		return;
	}
	
	//SensorFactory factory;
	//shield.clearAllSensors(); // serve per inizializzare
	for (int i = 0; i < sensorCount; i++) {

		logger.print(tag, "\n\n\t SENSOR: #" + String(i));

		JSONObject jObject;
		index += eprom.readJSON(index, &jObject);

		logger.print(tag, "\n: " + logger.formattedJson(jObject.toString()));

		Sensor* sensor = SensorFactory::createSensor(&jObject);
		
		if (sensor != nullptr) {
			logger.print(tag, "\n\n\t sensor=" + sensor->toString());
			shield.sensorList.add(sensor);
		}
	}
	
	logger.println(tag, "<<read Sensors\n");
}

void initEPROM()
{
	logger.println(tag, ">>initEPROM");

	EEPROM.begin(4096);

	byte id = EEPROM.read(ID_ADDR); // read the first byte from the EEPROM
	if (id == EEPROM_ID)
	{
		readEPROM();
	}
	else
	{
		writeEPROM();
	}
	logger.println(tag, "<<initEPROM");
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

bool testWifi() {

	Serial.println();
	Serial.println();
	Serial.print("Connecting");

	WiFi.begin();

	WiFi.mode(WIFI_STA);

	WiFi.disconnect();
	delay(100);

	//delay(2000);
	//WiFi.begin(networkSSID, networkPassword);
	//logger.print(tag, "\n\t WiFi.siid=" + WiFi.SSID());
	//logger.print(tag, "\n\t WiFi.pass=" + WiFi.RSSI());


	/*String pass = Shield::getNetworkSSID();
	String siid = Shield::getNetworkSSID();*/
	logger.print(tag, "\nCONNECTING\n");
	logger.print(tag, "siid=" + siid);
	logger.print(tag, "--" + pass);
	logger.print(tag, "--\n");
	WiFi.begin(siid.c_str(), pass.c_str());
	//WiFi.begin(ssidtest, passwordtest);
	
	int count = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
		if (count++ > 15) {
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


void callback(char* topic, byte* payload, unsigned int length) {
	
	logger.print(tag, "\n\n");
	logger.println(tag, ">>callback");
	logger.print(tag, "\n\t Message received");
	logger.print(tag, "\n\t topic=" + String(topic));
	
	String message = "";
	for (int i = 0; i < length; i++) {
		message += char(payload[i]);
	}
	logger.print(tag, "\n\t message=" + message);
	parseMessageReceived(String(topic), message);
	logger.println(tag, "<<callback");
}

void parseMessageReceived(String topic, String message) {

	String str = String("fromServer/shield/") + Shield::getMACAddress() + String("/registerresponse");
	//logger.print(tag, "\n\t str=" + str);
	String serverRequest = "fromServer/shield/" + String(Shield::getShieldId());

	if (topic.equals(str))
	{
		int id = message.toInt();
		logger.print(tag, "\n\t received registerresponse id=" + id);
		if (id > 0) {
			Shield::setShieldId(id);
			logger.print(tag, "\n\t Shieldid=" + String(Shield::getShieldId()));
		}
		return;

	}
	else if (topic.equals(serverRequest) &&
		message.equals("updatesettingstatusrequest")) {
		logger.print(tag, "\n\t received  setting status update request");
		//String response = shield.getSettingsJson();
		Command command;
		command.sendSettingsStatus(shield);
	}
	else if (topic.equals(serverRequest) &&
		message.equals("updatesensorstatusrequest")) {
		logger.print(tag, "\n\t received sensor status update request");
		//String response = shield.getSensorsStatusJson();
		String json = shield.getSensorsStatusJson();
		Command command;
		command.sendSensorsStatus(json);
	}
	else if (topic.equals(serverRequest + String("/postcommand"))) {
		logger.print(tag, "\n\t received command " + message);
		//String response = shield.getSensorsStatusJson();
		shield.receiveCommand(message);
	}
	else if (message.equals("start")) {
		logger.print(tag, "\n\t START!!!!!!!!!!!!!!!!");
		Shield::setMQTTMode(true);
	}
	else if (message.equals("stop")) {
		logger.print(tag, "\n\t STOP!!!!!!!!!!!!!!!!");
		Shield::setMQTTMode(false);
		mqttclient.disconnect();
	}
	else {
		logger.print(tag, "\n\t PARSE NOT FOUND");
	}

}

void reconnect() {
	logger.print(tag, "\n");
	logger.println(tag, ">>reconnect");
	// Loop until we're reconnected
	while (!mqttclient.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		String clientId = "ESP8266Client" + Shield::getMACAddress();
		if (mqttclient.connect(clientId)) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			//mqttclient.publish("send"/*topic.c_str()*/, "hello world");
			// ... and resubscribe
			mqttclient.subscribe("fromServer/#"/*"inTopic"*//*topic.c_str()*/);
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(mqttclient.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
	logger.println(tag, ">>reconnect");
}

void loadCredentials(int index) {

	logger.println(tag, ">>loadCredentials");

	MyEPROMClass eprom; 
	eprom.readString(index, &siid);
	eprom.readString(index+32, &pass);
	logger.print(tag, "\n\t siid=" + siid);
	logger.print(tag, "\n\t pass=" + pass);

	logger.println(tag, "<<loadCredentials");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials(int index) {

	logger.println(tag, ">>saveCredentials");
	MyEPROMClass eprom;
	eprom.writeString(index, &siid);
	eprom.writeString(index+32, &pass);

	logger.println(tag, "<<saveCredentials");
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
	String str = "\n\nstarting.... Versione 2.0";
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

	logger.print(tag, "\n\t siid=" + siid);
	logger.print(tag, "\n\t pass=" + pass);
	
	//shield.drawString(0, 80, "init onewire", 1, ST7735_WHITE);
	//shield.drawString(0, 40, "connecting to WiFi", 1, ST7735_WHITE);
	
	// Connect to WiFi network
	if (testWifi()) {

		//checkOTA();

		JSONObject json;
		String settings = getSettingsFromServer();
		loadSensors(settings);
		writeSettings();
		writeSensors();

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

		//readSensors();

		//shield.drawString(50, 70, String(shield.MAC_char), 1, ST7735_RED);
		//shield.drawString(60, 80, "DONE", 1, ST7735_WHITE);
		//shield.addActuators();

		IPAddress ipaddrgoogle;	
		Serial.println("\n");
		WiFi.hostByName(googleServerName, ipaddrgoogle);
		Serial.print(googleServerName);
		Serial.print(": ");
		Serial.println(ipaddrgoogle);
		
		IPAddress ipaddr;
		WiFi.hostByName(giacomohomeServerName, ipaddr);
		Serial.print(giacomohomeServerName);
		Serial.print(": ");
		Serial.println(ipaddr);

		IPAddress ipaddr2;
		WiFi.hostByName(giacomoboxServerName, ipaddr2);
		Serial.print(giacomoboxServerName);
		Serial.print(": ");
		Serial.println(ipaddr2);
		
		char ipaddress[255];
		sprintf(ipaddress,"%d.%d.%d.%d\n", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
		Serial.print("ipaddres= ");
		Serial.println(ipaddress);

		mqttclient.init(&client);
		mqttclient.setServer(Shield::getServerName(), 1883);
		mqttclient.setCallback(callback);
		reconnect();
		
	}
	else {
		logger.println(tag, "\n\n\tIMPOSSIBILE COLLEGARSI ALLA RETE\n");
		
		Shield::setMQTTMode(false);

		setupAP();

		server.begin();
		logger.print(tag, "\nLocal server started...192.168.4.1");
		shield.drawString(0, 60, "not connected. Local server 192.168.4.1", 1, ST7735_WHITE);
		// Print the IP address
		wdt_disable();
		wdt_enable(WDTO_8S);

		Shield::setMQTTMode(false);
	}

	/*logger.print(tag, "\n\t lastRestartDate=" + Shield::getLastRestartDate());
	if (Shield::getLastRestartDate().equals("") == true) {
		Shield::setLastRestartDate(Logger::getStrDate());
		logger.print(tag, "\n\t *lastRestartDate=" + Shield::getLastRestartDate());
	}

	shield.drawString(0, 80, "restarted", 1, ST7735_WHITE);

	Command commannd;
	commannd.sendRestartNotification();

	shield.clearScreen();
	logger.print(tag, "\n\t <<setup");*/
	shield.sensorList.show();

	logger.println(tag, "<<setup");
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
	String jsonResult = shield.receiveCommand(str);

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
		//shield.phearterActuator->setRemote(temperature);
	}
	String result = "";
	result += getJsonStatus(request, response);

	logger.print(tag, "\n\t <<setRemoteTemperature: " + result);
	return result;
}

String showPower(HttpRequest request, HttpResponse response) {
#ifdef dopo
	logger.print(tag, F("\n\n\t >>showPower "));

	logger.print(tag, F("\n\tprogramSettings.currentStatus="));
	//logger.print(tag, shield.phearterActuator->getStatus());

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
#endif
	return "";
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

// chiamata da jscript settings.html
String getJsonSettings(HttpRequest request, HttpResponse response) {
	logger.print(tag, F("\n\t >> getJsonSettings"));

	String data;
	data += "";
	data += shield.getSettingsJson();

	logger.print(tag, "\n\t << getJsonSettings" + data + "\n");
	return data;
}

void processNextPage() {

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
			else if (page.equalsIgnoreCase("sensorstatus")) {
				response.send(response.HTTPRESULT_OK, ESPWebServer::jsonContentType, getJsonSensorsStatus(request, response));
			}
			else if (page.equalsIgnoreCase("reset")) {
				// la chiamara a  send è dentro software reset perchè il metodo non torna mai
				softwareReset(request, response);
			}
			else if (page.equalsIgnoreCase("startmqtt")) {
				// la chiamara a  send è dentro software reset perchè il metodo non torna mai
				Shield::setMQTTMode(true);
				reconnect();
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

		logger.println(tag, "IP ADDRESS ERROR - re-register shield\n");
		shield.registerShield();
		shield.localIP = WiFi.localIP().toString();// a cosa serve??
		return;
	}

}

bool mqtt_publish(String topic, String message) {

	logger.print(tag, "\n");
	logger.println(tag, ">>mqtt_publish");

	logger.print(tag, "\n Message: [" + topic + String("] ") + message);
	bool res = mqttclient.publish(topic.c_str(), message.c_str());
	// qui bisognerebbe aggiungere qualche logica per gestire errore
	if (res)
		logger.print(tag, "\n\t message sent\n");
	else
		logger.print(tag, "\n\t message NOT sent!!!\n");

	logger.println(tag, "<<mqtt_publish\n");
	return res;
}

char msg[50];  //// DA ELIMINARE

void checkForSWUpdate() {

	logger.println(tag, ">>checkForSWUpdate");
	delay(2000);

	//t_httpUpdate_return ret = ESPhttpUpdate.update("http://192.168.1.3:8080//webduino/ota",Shield::swVersion);
	String updatePath = "http://" + Shield::getServerName() + ":" + Shield::getServerPort() + "//webduino/ota";
	logger.print(tag, "check for sw update " + updatePath);
	t_httpUpdate_return ret = ESPhttpUpdate.update(updatePath, Shield::swVersion);

	switch (ret) {
	case HTTP_UPDATE_FAILED:

		logger.print(tag, "\n\tHTTP_UPDATE_FAILD Error " + String(ESPhttpUpdate.getLastError()) + " " + ESPhttpUpdate.getLastErrorString().c_str());

		break;

	case HTTP_UPDATE_NO_UPDATES:
		logger.print(tag, "\n\tHTTP_UPDATE_NO_UPDATES");
		break;

	case HTTP_UPDATE_OK:
		logger.print(tag, "\n\tHTTP_UPDATE_OK");
		break;
	}
	logger.println(tag, "<<checkForSWUpdate");
}

void loop()
{
	//ArduinoOTA.handle();  // questa chiamata deve essere messa in loop()

	if (checkHTTPUpdate) {
		checkHTTPUpdate = false;
		checkForSWUpdate();
	}


	wdt_enable(WDTO_8S);

	if (Shield::getMQTTmode()) {

		//logger.println(tag, "debug");
		
		if (!client.connected()) {
			reconnect();
		}
		mqttclient.loop();
	
	}

	if (!Shield::getMQTTmode()) {
		processNextPage();
	}

	shield.checkStatus();


	unsigned long currMillis = millis();
	if (currMillis - lastTimeSync > timeSync_interval) {
		updateTime();
	}

	//#ifdef dopo
	if (Shield::getMQTTmode()) {

		/*unsigned long currMillis = millis();
		unsigned long timeDiff = currMillis - lastFlash;
		if (timeDiff > flash_interval) {

			lastFlash = currMillis;
			if (shield.id <= 0) {
				logger.println(tag, F("ID NON VALIDO\n"));
				shield.registerShield();
			}
			return;
		}*/

		

		/*if (currMillis - lastSendLog > SendLog_interval) {
			lastSendLog = currMillis;
			//logger.send();
			return;
		}*/
	}

}

bool updateTime() {
	logger.println(tag, "\n");
	logger.println(tag, ">>updateTime");

	unsigned long currMillis = millis();
	Command command;
	lastTimeSync = currMillis;
	command.timeSync();

	logger.println(tag, "<<updateTime");
	return true;
}
