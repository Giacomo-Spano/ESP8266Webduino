//#define ESP8266


#if defined ESP8266
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif

//needed for library
#include <DNSServer.h>
#if defined ESP8266
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>      

#include <PubSubClient.h>
#include "stdafx.h"

#include "ObjectClass.h"
#include "OnewireSensor.h"
#include "JSONArray.h"
#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#ifdef ESP8266
#include "TFTDisplay.h"
#endif
//#include "HttpResponse.h"
//#include "ESPWebServer.h"
//#include "HttpRequest.h"
#include "ESP8266Webduino.h"
//#include <ESP8266WiFi.h>



#include "MQTTClientClass.h"
#include "MyEPROMClass.h"

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
//#include "wol.h"
#include "Logger.h"
//#include "HttpHelper.h"
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
//#include <ESP8266HTTPClient.h>
#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#else
#include <ESP32httpUpdate.h>
#endif

#ifdef ESP8266
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif


bool checkHTTPUpdate = true; //true;
//

WiFiManager wifiManager;

#define production "1"
//#define wifibox "1"


/*extern "C" {
#include "user_interface.h"
}*/


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


bool mqttServerNotFoundError = false;
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
extern void resetWiFiManagerSettings();
extern void writeSettings();
extern void readSettings(JSONObject *json);
//extern void startIRreceiveLoop(void* callback/*IRrecv *pirrecv*/);
extern void startIRreceiveLoop(void(*callback)(void));
extern void stopIRreceiveLoop();

bool updateTime();


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
unsigned long lastCommandFailed = 0;
const int commandFailed_interval = 60000 * 30;// *12;// 60 secondi * 15 minuti
unsigned long lastReboot = 0;
const int reboot_interval = 3600000 * 24;// 24 ore

extern int __bss_end;
extern void *__brkval;

int sendRestartNotification = 0;
//bool shieldRegistered = false; // la shield si registra all'inizio e tutte le volte che va offline

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

void resetWiFiManagerSettings() {
	logger.print(tag, "\n\n\t >>resetWiFiManagerSettings");
	wifiManager.resetSettings();
	logger.println(tag, "\n\t <<resetWiFiManagerSettings");
}

void writeSettings() {

	logger.print(tag, "\n");
	logger.println(tag, "\n\t >>writeSettings");

	JSONObject json;

	// server name
	String serverName = Shield::getServerName();
	logger.print(tag, "\n\t serverName = " + serverName);
	json.pushString("server", serverName);

	// server port
	int serverport = Shield::getServerPort();
	logger.print(tag, "\n\t serverport = " + String(serverport));
	json.pushInteger("serverport", serverport);

	// mqtt server name
	String mqttServer = Shield::getMQTTServer();
	logger.print(tag, "\n\t mqttServer = " + mqttServer);
	json.pushString("mqttserver", mqttServer);

	// mqtt server port
	int mqttport = Shield::getMQTTPort();
	logger.print(tag, "\n\t mqttport = " + String(mqttport));
	json.pushInteger("mqttport", mqttport);

	// config mode
	bool configmode = Shield::getConfigMode();
	logger.print(tag, "\n\t configmode = " + String(configmode));
	json.pushBool("configmode", configmode);

	// resetsettings
	bool resetsettings = Shield::getResetSettings();
	logger.print(tag, "\n\t resetsettings = " + String(resetsettings));
	json.pushBool("resetsettings", resetsettings);

	// shieldName name
	/*String shieldname = Shield::getShieldName();
	logger.print(tag, "\n\t shieldname = " + shieldname);
	json.pushString("shieldname", shieldname);*/

	MyEPROMClass eprom;
	//logger.print(tag, "\n" + logger.formattedJson(json.toString()));
	eprom.writeJSON(SETTINGS_ADDR, &json);

	logger.println(tag, "<<writeSettings\n");
}

bool requestSettingsFromServer(String *settings) {

	logger.println(tag, ">>requestSettingsFromServer");

	Command command;
	return command.requestShieldSettings(settings);
}

void readSettings(JSONObject *json) {

	logger.println(tag, "\n\t >>readSettings");

	// server
	String server = "";
	if (json->has("server"))
		server = json->getString("server");
	logger.print(tag, "\n\t server=" + server);
	Shield::setServerName(server);

	// server port
	int serverport = 0;
	if (json->has("serverport"))
		serverport = json->getInteger("serverport");
	logger.print(tag, "\n\t serverport=" + String(serverport));
	Shield::setServerPort(serverport);

	// mqtt server
	String mqttserver = "";
	if (json->has("mqttserver"))
		mqttserver = json->getString("mqttserver");
	logger.print(tag, "\n\t mqttserver=" + mqttserver);
	Shield::setMQTTServer(mqttserver);

	// mqtt port
	int mqttport = 0;
	if (json->has("mqttport"))
		mqttport = json->getInteger("mqttport");
	logger.print(tag, "\n\t mqttport=" + String(mqttport));
	Shield::setMQTTPort(mqttport);

	// config mode
	bool configmode = false;
	if (json->has("configmode"))
		configmode = json->getBool("configmode");
	logger.print(tag, "\n\t configmode=" + String(configmode));
	Shield::setConfigMode(configmode);

	// resetsettings
	bool resetsettings = false;
	if (json->has("resetsettings"))
		resetsettings = json->getBool("resetsettings");
	logger.print(tag, "\n\t resetsettings=" + String(resetsettings));
	Shield::setResetSettings(resetsettings);

	//Shield::setServerName("giacomohome.ddns.net");
	// Shield::setMQTTServer("giacomohome.ddns.net");
	/*Shield::setServerName("192.168.1.3");
	Shield::setMQTTServer("192.168.1.3");

	Shield::setServerPort(8080);
	Shield::setMQTTPort(1883);*/
	//writeSettings();

	logger.print(tag, "\n\t <<readSettings");
}

void writeEPROM() {

	logger.print(tag, "\n\n\t >>write EPROM");

	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data

	MyEPROMClass eprom;
	eprom.writeInt(SWVERSION_ADDR, EPROM_Table_Schema_Version);

	writeSettings();

	writeSensors();

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

#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266

		logger.print(tag, "\n\n\t sensor#: " + String(i));
		Sensor* sensor = (Sensor*)shield.sensorList.get(i);
		JSONObject json2;
		sensor->getJSON(&json2);
		//logger.print(tag, "\n" + logger.formattedJson(json2.toString()));

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
	eprom.readInt(SWVERSION_ADDR, &epromversion);
	logger.print(tag, "\n\t epromversion=" + String(epromversion));

	// load settings
	JSONObject json;
	eprom.readJSON(SETTINGS_ADDR, &json);
	logger.print(tag, "\n\n\t json:" + logger.formattedJson(json.toString()));
	readSettings(&json);

	logger.print(tag, "\n\n\t <<read EPROM\n");
}

// carica i settings dal server
bool loadSensors(String settings) {

	//logger.print(tag, "\n\n");
	logger.print(tag, "\n\t>>loadSensors settings=" + settings/*logger.formattedJson(settings)*/);
	logger.printFreeMem(tag, "");

	JSONObject json(settings);

	if (json.has("shieldid")) {
		int id = json.getInteger("shieldid");
		Shield::setShieldId(id);

	}
	else {
		logger.println(tag, "error -ID MISSING");
		return false;
	}

	if (json.has("name")) {
		String name = json.getString("name");
		Shield::setShieldName(name);
	}

	logger.printFreeMem(tag, "");
	if (json.has("sensors")) {
		String str = json.getJSONArray("sensors");
		JSONArray sensors(str);
		logger.printFreeMem(tag, "");
		String sensorstr = sensors.getFirst();
		while (sensorstr != "") {
			logger.printFreeMem(tag, "sensor start");
#ifdef ESP8266
			ESP.wdtFeed();
#endif // ESP8266
			logger.print(tag, "\n\n\t SENSOR: " + sensorstr);
			JSONObject jObject(sensorstr);
			logger.print(tag, "\n: " + logger.formattedJson(jObject.toString()));

			Sensor* sensor = SensorFactory::createSensor(&jObject);

			if (sensor != nullptr) {
				logger.print(tag, "\n\n\t sensor=" + sensor->toString());
				shield.sensorList.add(sensor);
			}

			sensorstr = sensors.getNext();
			logger.printFreeMem(tag, "sensor end");
		}

	}
	logger.printFreeMem(tag, "end loadsensor");
	logger.println(tag, "<<loadSensors\n");
	return true;

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
#ifdef ESP8266
			logger.println(tag, (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
#endif
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
#ifdef ESP8266
		st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
#endif
		st += "</li>";
	}
	st += "</ul>";
	delay(100);

	logger.println(tag, "input SSID");
	bool readInput = true;
	while (readInput) {
		while (Serial.available()) {
			siid = Serial.readString();
			readInput = false;
		}
	}
	logger.println(tag, siid);

	logger.println(tag, "input pass");
	readInput = true;
	while (readInput) {
		while (Serial.available()) {
			pass = Serial.readString();
			readInput = false;
		}
	}
	logger.println(tag, pass);

	//saveCredentials(CREDENTIAL_ADDR);

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

bool shouldSaveConfig = true;

void saveConfigCallback() {
	Serial.println("Should save config");
	shouldSaveConfig = true;

}

bool testWifi() {

	// The extra parameters to be configured (can be either global or just in the setup)
	// After connecting, parameter.getValue() will get you the configured value
	// id/name placeholder/prompt default length
	String server = Shield::getServerName();
	String serverport = String(Shield::getServerPort());
	String mqttserver = Shield::getMQTTServer();
	String mqttport = String(Shield::getMQTTPort());

	WiFiManagerParameter custom_server("server", "server", server.c_str(), 40);
	WiFiManagerParameter custom_server_port("port", "port", serverport.c_str(), 5);
	WiFiManagerParameter custom_mqtt_server("mqttserver", "mqtt server", mqttserver.c_str(), 40);
	WiFiManagerParameter custom_mqtt_port("mqttport", "mqtt port", mqttport.c_str(), 5);

	// put your setup code here, to run once:
	//WiFiManager wifiManager;

	//set config save notify callback
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	// Uncomment and run it once, if you want to erase all the stored information
	//wifiManager.resetSettings();
	if (Shield::getResetSettings()) {
		wifiManager.resetSettings();
		Shield::setResetSettings(false);
	}

	//add all your parameters here
	wifiManager.addParameter(&custom_server);
	wifiManager.addParameter(&custom_server_port);
	wifiManager.addParameter(&custom_mqtt_server);
	wifiManager.addParameter(&custom_mqtt_port);

	WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
	wifiManager.addParameter(&custom_text);
	//wifiManager.addParameter(&custom_blynk_token);

	wifiManager.setConfigPortalTimeout(180);

	//first parameter is name of access point, second is the password
	//wifiManager.autoConnect("AP-NAME", "passwd");

	//wifiManager.autoConnect("AP-NAME");
	//or if you want to use and auto generated name from 'ESP' and the esp's Chip ID use
	//wifiManager.autoConnect();
	if (!wifiManager.autoConnect()) {
		Serial.println("failed to connect and hit timeout");
		delay(3000);
		//reset and try again, or maybe put it to deep sleep
#ifdef ESP8266
		ESP.reset();
#endif
		delay(5000);
		//return false;
	}

	//if you get here you have connected to the WiFi
	Serial.println("connected...yeey :)");

	if (shouldSaveConfig) {

		Shield::setServerName(custom_server.getValue());

		int serverport = atoi(custom_server_port.getValue());
		Shield::setServerPort(serverport);

		Shield::setMQTTServer(custom_mqtt_server.getValue());

		int mqttport = atoi(custom_mqtt_port.getValue());
		Shield::setMQTTPort(mqttport);

		writeSettings();
	}

	//read updated parameters
	Serial.println(custom_server.getValue());
	Serial.println(custom_server_port.getValue());
	Serial.println(custom_mqtt_server.getValue());
	Serial.println(custom_mqtt_port.getValue());

	// if you get here you have connected to the WiFi
	Serial.println("Connected.");

	return true;
}



void messageReceived(char* topic, byte* payload, unsigned int length) {

	logger.print(tag, "\n");
	logger.println(tag, ">>messageReceived");
	logger.print(tag, "\n\t topic=" + String(topic));


	if (ESP.getFreeHeap() < 2000) {
		logger.println(tag, "\n\n\n\LOW MEMORY");
		logger.printFreeMem(tag, "");
		logger.println(tag, "\n\n\n\LOW MEMORY");
		delay(1000);
		logger.printFreeMem(tag, "");
		return;
	}

	String message = "";
	for (int i = 0; i < length; i++) {
		message += char(payload[i]);
	}
	logger.print(tag, "\n\t message=" + message);
	parseMessageReceived(String(topic), message);
	logger.println(tag, "<<messageReceived");
}

void parseMessageReceived(String topic, String message) {

	logger.print(tag, "\n\t >>parseMessageReceived");
	String str = String("fromServer/shield/") + Shield::getMACAddress();

	if (topic.equals(str + "/settings")) {
		logger.print(tag, "\n\t received shield settings\n" + message + "\n");
		loadSensors(message);
		writeEPROM();

	}
	else if (topic.equals(str + "/reboot")) {
		logger.print(tag, "\n\t received reboot request");
		ESP.restart();
	}
	else if (topic.equals(str + "/resetsettings")) {
		logger.print(tag, "\n\t received resetsettings request");
		Shield::setResetSettings(true);
		writeEPROM();
		ESP.restart();
	}
	else if (topic.equals(str + "/time")) { // risposta a time
		logger.print(tag, "\n\t received time");
		time_t t = (time_t)atoll(message.c_str());
		logger.print(tag, "\n\t time=" + String(t));
		setTime(t);
	}
	else if (topic.equals(str + "/command")) {
		logger.print(tag, "\n\t received command " + message);
		shield.receiveCommand(message);
	}
	else if (str.equals("configmode")) {
		logger.print(tag, "\n\t CONFIG MODE!!!!!!!!!!!!!!!!");
		Shield::setConfigMode(true);
	}
	else if (str.equals("reset")) {
		logger.print(tag, "\n\t RESET SETTINGS!!!!!!!!!!!!!!!!");
		Shield::setResetSettings(true);
		resetEPROM();
		ESP.restart();
	}
	else if (str.equals("startmqtt")) {
		logger.print(tag, "\n\t START!!!!!!!!!!!!!!!!");
		Shield::setMQTTMode(true);
	}
	else if (str.equals("stopmqtt")) {
		logger.print(tag, "\n\t STOP!!!!!!!!!!!!!!!!");
		Shield::setMQTTMode(false);
		mqttclient.disconnect();
	}
	else {
		logger.print(tag, "\n\t PARSE NOT FOUND");
	}
	logger.print(tag, "\n\t <<parseMessageReceived");
}

void reconnect() {
	logger.print(tag, "\n");
	logger.println(tag, ">>reconnect");
	// Loop until we're reconnected
	while (!mqttclient.connected()) {
		Serial.print("\nAttempting MQTT connection...");
		// Attempt to connect
		String clientId = "ESP8266Client" + Shield::getMACAddress();
		if (mqttclient.connect(clientId)) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			//mqttclient.publish("send"/*topic.c_str()*/, "hello world");
			String topic = "fromServer/shield/" + Shield::getMACAddress() + "/#";
			logger.print(tag, "\n\t Subscribe to topic:" + topic);
			mqttclient.subscribe(topic.c_str());
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

void setup()
{
	Serial.begin(9600);
	delay(10);

	lastReboot = millis();
	shield.lastCheckHealth = millis();

	Logger::init();
	logger.print(tag, "\n\t >>setup");
	logger.print(tag, "\n\n *******************RESTARTING************************");

	shield.init();
#ifdef ESP8266
	shield.drawString(0, 0, "restarting..", 1, ST7735_WHITE);
#endif

	// Initialising the UI will init the display too.
	String str = "\n\nstartingx.... Versione ";
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
#ifdef ESP8266
	shield.drawString(0, 20, "read eprom..", 1, ST7735_WHITE);
#endif

	initEPROM();

	// disabilita il watchdog sw e abilita quello hw
#ifdef ESP8266
	ESP.wdtDisable();
#endif

	// Connect to WiFi network
	if (testWifi()) {



		mqttclient.init(&client);
		mqttclient.setServer(Shield::getMQTTServer(), Shield::getMQTTPort());
		mqttclient.setCallback(messageReceived);
		reconnect();

		updateTime();
		//Logger::sendLogToServer();

		JSONObject json;
		String settings;
		bool res = requestSettingsFromServer(&settings);

		if (!res) {
			logger.println(tag, "\n\n\tIMPOSSIBILE TROVARE IL SERVER MQTT\n");
			mqttServerNotFoundError = true;
		}
		else {
#ifdef ESP8266
			shield.drawString(0, 50, "connected..Start server", 1, ST7735_WHITE);
#endif
			// Start the server
			server.begin();
			logger.print(tag, "Server started");

#ifdef ESP8266
			shield.drawString(0, 60, "server started", 1, ST7735_WHITE);
#endif

			shield.localIP = WiFi.localIP().toString();
			// Print the IP address
			logger.println(tag, shield.localIP);

		}

	}
	else {
		mqttclient.init(&client);
		mqttclient.setServer("192.168.4.2", 1883);
		mqttclient.setCallback(messageReceived);
		reconnect();

	}

	shield.sensorList.show();

	Command command;
	command.sendShieldStatus(shield.getJson());

	//ESP.wdtDisable();
	

	logger.println(tag, "<<setup");
}






bool _mqtt_publish(char* topic, char* payload) {

	logger.print(tag, "\n");
	logger.println(tag, ">>_mqtt_publish \n");

	logger.print(tag, "topic:");
	logger.print(tag, topic);
	logger.print(tag, "\npayload:");
	logger.print(tag, payload);

	if (strlen(payload) >= MQTT_MAX_PACKET_SIZE) {
		logger.print(tag, "\n\t payload TOO BIG!!!\n");
	}

	//logger.print(tag, "\n Message: [" + String(topic) + String("] ") + payload);
	bool res = mqttclient.publish(topic, payload);
	// qui bisognerebbe aggiungere qualche logica per gestire errore
	if (res) {
		logger.print(tag, "\n\t payload sent\n");
	}
	else {
		lastCommandFailed = millis();
		logger.print(tag, "\n\t payload NOT sent!!!\n");
	}

	logger.println(tag, "<<_mqtt_publish payload\n");
	return res;
}


bool mqtt_publish(String topic, String message) {


	logger.print(tag, "\n\t >>mqtt_publish");
	logger.printFreeMem(tag, "--mqtt_publish");


	logger.print(tag, "\n\t topic: ");
	logger.print(tag, topic);
	logger.print(tag, "\n\t message: ");
	logger.print(tag, message);

	//logger.print(tag, "\n Message: [" + topic + String("] ") + message);
	bool res = mqttclient.publish(topic.c_str(), message.c_str());
	// qui bisognerebbe aggiungere qualche logica per gestire errore
	if (res) {
		logger.print(tag, "\n\t payload sent");
	}
	else {
		lastCommandFailed = millis();
		logger.print(tag, "\n\t payload NOT sent!!!\n");
	}
	logger.print(tag, "\n\t <<mqtt_publish");
	logger.printFreeMem(tag, "--mqtt_publish");
	return res;
}

char msg[50];  //// DA ELIMINARE

void checkForSWUpdate() {

#ifdef ESP8266

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

#endif
}

bool IRReceiving = false;
//int(*foo_p)(int) = &foo;
void(*callbackfunction)(void) ;

void loop()
{
#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266
	if (ESP.getFreeHeap() < 2000) {
		logger.println(tag, "\n\n\n\LOW MEMORY");
		logger.printFreeMem(tag, "");
		delay(1000);
		logger.println(tag, "\n\n\n\LOW MEMORY");
		logger.printFreeMem(tag, "");
		
		return;
	}

	

	if (IRReceiving) {
		//IRreceiveLoop();
		callbackfunction();
		//return;
	}

	//for(;;) {}

	String prova = "";
	if (Serial.available()) {
		logger.println(tag, "\n\n\n\-----------READINPUT--------\n\n");
		prova = Serial.readString();
		logger.println(tag, prova);
	}

	unsigned long currMillis = millis();
	if (currMillis - lastReboot > reboot_interval) {
		logger.println(tag, "\n\n\n\-----------lastReboot TIMEOUT REBOOT--------\n\n");
		ESP.restart();
	}


	currMillis = millis();
	if (currMillis - shield.lastCheckHealth > shield.checkHealth_timeout) {
		logger.println(tag, "\n\n\n\-----------CHECK HEALTH TIMEOUT REBOOT--------\n\n");
		ESP.restart();
	}

	if (checkHTTPUpdate) {
#ifdef ESP8266
		ESP.wdtFeed();
#endif
		checkHTTPUpdate = false;
		checkForSWUpdate();
	}

#ifdef ESP8266
	wdt_enable(WDTO_8S);
#endif

	if (Shield::getConfigMode()) {

		logger.println(tag, "start config mode....");

		if (!wifiManager.startConfigPortal("OnDemandAP")) {
			Serial.println("failed to connect and hit timeout");
			delay(3000);
			//reset and try again, or maybe put it to deep sleep
			logger.println(tag, "\n\n\n\-----------failed to connect and hit timeout REBOOT--------\n\n");
#ifdef ESP8266
			ESP.reset();
#endif
			delay(5000);
		}
		Shield::setConfigMode(false);
		logger.println(tag, "end config mode...");
	}


	// questo serve per controllare se ikl client mqtt è connesso
	// ma in qualche caso secondo me non funziona
	if (Shield::getMQTTmode()) {
		if (!client.connected()) {
			reconnect();
		}
		mqttclient.loop();

	}

	if (!Shield::getMQTTmode()) {
		//processNextPage();
	}

	//logger.println(tag, "before checkStatus");
	shield.checkStatus();
	//logger.println(tag, "after checkStatus");
	
	
	// questo si potrebbe eliminare, tanto dl'ora non è usata
	currMillis = millis();
	if (currMillis - lastTimeSync > timeSync_interval) {
		updateTime();
	}

	currMillis = millis();
	if (lastCommandFailed > 0 && currMillis - lastCommandFailed > commandFailed_interval) {
		logger.println(tag, "\n\n\n\-----------lastCommandFailed TIMEOUT REBOOT--------\n\n");
		ESP.restart();
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


// questa funzione non è usata
bool __updateTime() {
	logger.println(tag, "\n\n\n\n\n\n\n\n\n\n\n\n");
	logger.println(tag, ">>updateTime");

	unsigned long currMillis = millis();
	Command command;
	lastTimeSync = currMillis;
	//command.timeSync();
	command.requestTime();

	logger.println(tag, "<<updateTime");
	return true;
}

bool updateTime() {

	logger.println(tag, ">>requestTimeFromServer");

	Command command;
	return command.requestTime();
}



void startIRreceiveLoop(void(*callback)(void)/*IRrecv *pirrecvptr*/) {

	IRReceiving = true;
	callbackfunction = callback;
	logger.println(tag, "\n\n\n READY TO RECEIVE\n\n");
}

extern void stopIRreceiveLoop() {
	IRReceiving = false;
	logger.println(tag, "\n\n\n STOP RECEIVE\n\n");
}

