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

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <EEPROM.h>
//#include "EEPROMAnything.h"
//#include "wol.h"
#include "Logger.h"
//#include "HttpHelper.h"
//#include "JSON.h"
#include "Shield.h"
#include "Command.h"
//#include "Program.h"
//#include "Actuator.h"
#include <Time.h>
#include "TimeLib.h"
#include "POSTData.h"

//#include "JSONObject.h"

#include <Wire.h>


#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include "Ticker.h"

//void triggerUpdateTime();


/*Ticker updatetimeTimer(triggerUpdateTime, 30000); // once, immediately
bool timeNotUpdated = true;

void triggerUpdateTime() {
	logger.println(tag, F("\n\t >triggerUpdateTime"));
	timeNotUpdated = true;
}



/*int sda = 0;
int scl = 1;
int x = 0;*/

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
bool mqttLoaded = false; //true;


//

WiFiManager wifiManager;

ESPDisplay espDisplay;

#define production "1"


int EPROM_Table_Schema_Version = 8;

Logger logger;

String tag = "Webduino";
static Shield shield;

// EPROM
const byte EEPROM_ID = 0x96; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int SWVERSION_ADDR = 1;
const int CREDENTIAL_ADDR = SWVERSION_ADDR + 10; // the EEPROM address used to store the pin
const int SETTINGS_ADDR = CREDENTIAL_ADDR + Shield::networkSSIDLen + Shield::networkPasswordLen + 2; // the EEPROM address used to store the pin
const int SENSOR_ADDR = SETTINGS_ADDR + 1000; // the EEPROM address used to store the pin
//int epromversion = 0;

extern void resetWiFiManagerSettings();
//extern void writeSettings();
//extern void readSettings(JSONObject *json);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;

extern bool mqtt_publish(String topic, String message);
MQTTClientClass mqttclient;

const int timeSync_interval = 60000 * 5;// *12;// 60 secondi * 15 minuti
unsigned long lastCommandFailed = 0;
const int commandFailed_interval = 60000 * 30;// *12;// 60 secondi * 15 minuti
unsigned long lastReboot = 0;
const int reboot_interval = 3600000 * 24;// 24 ore

extern int __bss_end;
extern void *__brkval;

#ifdef dopo
void resetEPROM() {
	// scrive zero nel primo settore della EPROM
	// così al riavvio successivo saranno ripristinati i valori di default
	logger.print(tag, F("\n\n\t >>resetEPROM"));

	byte hiByte;
	byte loByte;
	EEPROM.write(ID_ADDR, 0/*EEPROM_ID*/); // delete EEPROM_ID to indicate invalid data

	EEPROM.commit();
	logger.println(tag, F("\n\t <<resetEPROM"));
}
#endif

void resetWiFiManagerSettings() {
	logger.print(tag, F("\n\n\t >>resetWiFiManagerSettings"));
	wifiManager.resetSettings();
	logger.println(tag, F("\n\t <<resetWiFiManagerSettings"));
}

#ifdef dopo
void writeSettings() {

	logger.print(tag, F("\n"));
	logger.println(tag, "\n\t >>writeSettings");

	JSONObject json;

	// server name
	String serverName = shield.getServerName();
	//logger.print(tag, "\n\t serverName = " + serverName);
	json.pushString("server", serverName);

	// server port
	int serverport = shield.getServerPort();
	//logger.print(tag, "\n\t serverport = " + String(serverport));
	json.pushInteger("serverport", serverport);

	// mqtt server name
	String mqttServer = shield.getMQTTServer();
	logger.print(tag, "\n\t mqttServer = " + mqttServer);
	json.pushString("mqttserver", mqttServer);

	// mqtt server port
	int mqttport = shield.getMQTTPort();
	//logger.print(tag, "\n\t mqttport = " + String(mqttport));
	json.pushInteger("mqttport", mqttport);

	// config mode
	bool configmode = shield.getConfigMode();
	//logger.print(tag, "\n\t configmode = " + String(configmode));
	json.pushBool("configmode", configmode);

	// resetsettings
	bool resetsettings = shield.getResetSettings();
	//logger.print(tag, "\n\t resetsettings = " + String(resetsettings));
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
#endif

bool requestSettingsFromServer() {
	logger.println(tag, F(">>requestSettingsFromServer"));
	shield.setEvent(F("request server setting.."));
	Command command;
	bool res = command.requestShieldSettings(shield.getMACAddress());
	logger.println(tag, F("<<requestSettingsFromServer res="));
	logger.println(tag, Logger::boolToString(res));
	if (res) {
		shield.settingFromServerRequested = true;
		shield.settingRequestedTime = millis();
	}
	logger.println(tag, F("<<requestSettingsFromServer"));
	return res;
}

#ifdef dopo
void readSettings(JSONObject *json) {

	logger.println(tag, F("\n\t >>readSettings"));

	// server
	String server = "";
	if (json->has("server"))
		server = json->getString("server");
	//logger.print(tag, "\n\t server=" + server);
	shield.setServerName(server);

	// server port
	int serverport = 0;
	if (json->has("serverport"))
		serverport = json->getInteger("serverport");
	//logger.print(tag, "\n\t serverport=" + String(serverport));
	shield.setServerPort(serverport);

	// mqtt server
	String mqttserver = "";
	if (json->has("mqttserver"))
		mqttserver = json->getString("mqttserver");
	//logger.print(tag, "\n\t mqttserver=" + mqttserver);
	shield.setMQTTServer(mqttserver);

	// mqtt port
	int mqttport = 0;
	if (json->has("mqttport"))
		mqttport = json->getInteger("mqttport");
	//logger.print(tag, "\n\t mqttport=" + String(mqttport));
	shield.setMQTTPort(mqttport);

	// config mode
	bool configmode = false;
	if (json->has("configmode"))
		configmode = json->getBool("configmode");
	//logger.print(tag, "\n\t configmode=" + String(configmode));
	shield.setConfigMode(configmode);

	// resetsettings
	bool resetsettings = false;
	if (json->has("resetsettings"))
		resetsettings = json->getBool("resetsettings");
	//logger.print(tag, "\n\t resetsettings=" + String(resetsettings));
	shield.setResetSettings(resetsettings);

	//Shield::setServerName("giacomohome.ddns.net");
	// Shield::setMQTTServer("giacomohome.ddns.net");
	/*Shield::setServerName("192.168.1.3");
	Shield::setMQTTServer("192.168.1.3");

	Shield::setServerPort(8080);
	Shield::setMQTTPort(1883);*/
	//writeSettings();

	logger.print(tag, F("\n\t <<readSettings"));
}
#endif

#ifdef dopo
void writeEPROM() {

	logger.print(tag, F("\n\n\t >>write EPROM"));

	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data

	MyEPROMClass eprom;
	eprom.writeInt(SWVERSION_ADDR, EPROM_Table_Schema_Version);

	writeSettings();

	writeSensors();

	logger.print(tag, F("\n\t <<write EPROM\n"));
}
#endif

#ifdef dopo
void writeSensors() {

	logger.print(tag, F("\n"));
	logger.println(tag, F(">>writeSensors"));

	MyEPROMClass eprom;
	// sensor number
	int index = SENSOR_ADDR;
	index += eprom.writeInt(index, shield.sensorList.length());

	for (int i = 0; i < shield.sensorList.length(); i++) {

#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266

		logger.print(tag, F("\n\n\t sensor#: "));
		logger.print(tag, String(i));
		Sensor* sensor = (Sensor*)shield.sensorList.get(i);
		JSONObject json2;
		//sensor->getJSON(&json2);
		//logger.print(tag, "\n" + logger.formattedJson(json2.toString()));

		//index += eprom.writeJSON(index, &json2);
	}

	logger.println(tag, F("<<writeSensors\n"));
}
#endif

#ifdef dopo
void readEPROM() {

	logger.println(tag, F("\t >>read EPROM"));

	// check EPROM_ID
	byte epromID = EEPROM.read(ID_ADDR);
	logger.print(tag, F("\n\t epromID="));
	logger.print(tag, String(epromID));

	if (epromID != EEPROM_ID) { // reset EPROM_ID
		logger.print(tag, F("\n\t INVALID DATA"));
		logger.print(tag, String(epromID));
		writeEPROM();
		return;
	}
	MyEPROMClass eprom;
	// epromversion
	eprom.readInt(SWVERSION_ADDR, &epromversion);
	//logger.print(tag, "\n\t epromversion=" + String(epromversion));

	// load settings
	JSONObject json;
	eprom.readJSON(SETTINGS_ADDR, &json);
	//logger.print(tag, "\n\n\t json:" + logger.formattedJson(json.toString()));
	readSettings(&json);

	logger.print(tag, F("\n\n\t <<read EPROM\n"));
}
#endif

#ifdef dopo
void readSensors() {

	logger.println(tag, F(">>read Sensors"));

	int index = SENSOR_ADDR;
	MyEPROMClass eprom;
	int sensorCount;
	index += eprom.readInt(index, &sensorCount);
	logger.print(tag, F("\n\t sensorCount="));
	logger.print(tag, String(sensorCount));
	if (sensorCount < 0 || sensorCount > Shield::maxSensorNum) {
		logger.print(tag, F("\n\t INVALID VALUE!"));
		sensorCount = 0;
		return;
	}

	//SensorFactory factory;
	shield.clearAllSensors(); // serve per inizializzare
	for (int i = 0; i < sensorCount; i++) {

		//logger.print(tag, "\n\n\t SENSOR: #" + String(i));

		JSONObject jObject;
		index += eprom.readJSON(index, &jObject);

		logger.print(tag, F("\n: "));
		logger.print(tag, logger.formattedJson(jObject.toString()));

		// questo bisogna riscrivero con arduinojson
		/*Sensor* sensor = SensorFactory::createSensor(&jObject);

		if (sensor != nullptr) {
			logger.print(tag, "\n\n\t sensor=" + sensor->toString());
			shield.sensorList.add(sensor);
		}*/
	}

	logger.println(tag, F("<<read Sensors\n"));
}
#endif

#ifdef doipo
void initEPROM()
{
	logger.println(tag, F(">>initEPROM"));

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
	logger.println(tag, F("<<initEPROM"));
}
#endif

#ifdef dopo
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
#endif

#ifdef dopo
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
#endif

bool shouldSaveConfig = true;

void saveConfigCallback() {
	logger.println(tag, F("Should save config"));
	shouldSaveConfig = true;

}

bool testWifi() {

	// The extra parameters to be configured (can be either global or just in the setup)
	// After connecting, parameter.getValue() will get you the configured value
	// id/name placeholder/prompt default length
	String server = shield.getServerName();
	String serverport = String(shield.getServerPort());
	String mqttserver = shield.getMQTTServer();
	String mqttport = String(shield.getMQTTPort());

	WiFiManagerParameter custom_server("server", "server", server.c_str(), 40);
	WiFiManagerParameter custom_server_port("port", "port", serverport.c_str(), 5);
	WiFiManagerParameter custom_mqtt_server("mqttserver", "mqtt server", mqttserver.c_str(), 40);
	WiFiManagerParameter custom_mqtt_port("mqttport", "mqtt port", mqttport.c_str(), 5);

	// put your setup code here, to run once:
	//WiFiManager wifiManager;

	//set config save notify callback
	//wifiManager.setSaveConfigCallback(saveConfigCallback);

	// Uncomment and run it once, if you want to erase all the stored information
	//wifiManager.resetSettings();
	if (shield.getResetSettings()) {
		wifiManager.resetSettings();
		shield.setResetSettings(false);
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
		shield.setEvent(F("failed to connect and hit timeout"));
		shield.invalidateDisplay();
		logger.println(tag, F("failed to connect and hit timeout"));
		delay(3000);
		ESP.restart();
		
	}

	//if you get here you have connected to the WiFi
	logger.println(tag, F("connected..."));
	shield.setEvent(F("Wifi connected"));
	shield.invalidateDisplay();

	if (shouldSaveConfig) {

		shield.setServerName(custom_server.getValue());
		int serverport = atoi(custom_server_port.getValue());
		shield.setServerPort(serverport);
		shield.setMQTTServer(custom_mqtt_server.getValue());
		int mqttport = atoi(custom_mqtt_port.getValue());
		shield.setMQTTPort(mqttport);

		writeConfig();
		//writeSettings();
	}

	//read updated parameters
	Serial.println(custom_server.getValue());
	Serial.println(custom_server_port.getValue());
	Serial.println(custom_mqtt_server.getValue());
	Serial.println(custom_mqtt_port.getValue());

	return true;
}

void messageReceived(char* topic, byte* payload, unsigned int length) {

	//logger.print(tag, "\n");
	logger.println(tag, F(">>messageReceived"));
	logger.print(tag, F("\n\t topic="));
	logger.print(tag, String(topic));

	/*if (ESP.getFreeHeap() < 2000) {
		logger.println(tag, F("\n\n\n\LOW MEMORY"));
		logger.printFreeMem(tag, "");
		logger.println(tag, F("\n\n\n\LOW MEMORY"));
		delay(1000);
		logger.printFreeMem(tag, "");
		return;
	}*/
	String message = "";
	for (int i = 0; i < length; i++) {
		message += char(payload[i]);
	}
	//logger.print(tag, "\n\t message=" + message);
	shield.parseMessageReceived(String(topic), message);
	logger.println(tag, F("<<messageReceived"));
}


bool reconnect() {
	logger.print(tag, F("\n\n\t>>reconnect"));
	shield.setStatus(F("CONNECTING.."));
	// Loop until we're reconnected
	if (!mqttclient.connected()) {

		for (int i = 0; i < 3; i++) {
			logger.print(tag, F("\n\tAttempting MQTT connection..."));
			logger.print(tag, F("\n\ttemptative "));
			logger.print(tag, String(i));
			// Attempt to connect
			String clientId = "ESP8266Client" + shield.getMACAddress();
			if (mqttclient.connect(clientId)) {
				logger.print(tag, F("\n\tconnected"));
				// Once connected, publish an announcement...
				//mqttclient.publish("send"/*topic.c_str()*/, "hello world");
				String topic = "fromServer/shield/" + shield.getMACAddress() + "/#";
				logger.print(tag, F("\n\t Subscribe to topic:"));
				logger.print(tag, topic);
				mqttclient.subscribe(topic.c_str());
				shield.setStatus(F("ONLINE"));
				logger.print(tag, F("\n\t<<reconnect\n\n"));
				return true;
			}
			else {
				logger.print(tag, F("\n\tfailed, rc="));
				logger.print(tag, mqttclient.state());
				logger.print(tag, F("\n\ttry again in 1 seconds"));
				// Wait 1 seconds before retrying
				delay(1000);
			}
		}
	}
	shield.setStatus(F("OFFLINE"));
	logger.print(tag, F("\n\t<<reconnect FAILED\n\n"));
	return false;
}

void readConfig() {
	
	//clean FS, for testing
	//SPIFFS.format();

	//read configuration from FS json
	Serial.println("mounting FS...");

	if (SPIFFS.begin()) {
		Serial.println("mounted file system");
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			Serial.println("reading config file");
			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile) {
				Serial.println("opened config file");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					Serial.println("\nparsed json");

					if (json.containsKey("http_server")) {
						Serial.println("http_server: ");
						String str = json["http_server"];
						Serial.println(str);
						shield.setServerName(str);
					}
					if (json.containsKey("http_port")) {
						Serial.println("http_port: ");
						String str = json["http_port"];
						Serial.println(str);
						shield.setServerPort(json["http_port"]);
					}
					if (json.containsKey("mqtt_server")) {
						Serial.println("mqtt_server: ");
						String str = json["mqtt_server"];
						Serial.println(str);
						shield.setMQTTServer(json["mqtt_server"]);
					}
					if (json.containsKey("mqtt_port")) {
						Serial.println("mqtt_port: ");
						String str = json["mqtt_port"];
						Serial.println(str);
						shield.setMQTTPort(json["mqtt_port"]);
					}
					if (json.containsKey("mqtt_user")) {
						Serial.println("mqtt_user: ");
						String str = json["mqtt_user"];
						Serial.println(str);
						//shield.setMQTTUser(json["mqtt_user"]);
					}
					if (json.containsKey("mqtt_password")) {
						Serial.println("mqtt_password: ");
						String str = json["mqtt_password"];
						Serial.println(str);
						//shield.setMQTTPassword(json["mqtt_password"]);
					}
					if (json.containsKey("shieldid")) {
						Serial.println("shieldid: ");
						String str = json["shieldid"];
						Serial.println(str);
						//shield.set(json["shieldid"]);
					}
					// resetsettings
					if (json.containsKey("resetsettings")) {
						Serial.println("resetsettings: ");
						String str = json["resetsettings"];
						Serial.println(str);
						//shield.setResetSettings(json["resetsettings"]);
					}
				}
				else {
					Serial.println("failed to load json config");
					//clean FS, for testing
					SPIFFS.format();
					writeConfig();
					//ESP.restart();
				}
			}
		}
	}
	else {
		Serial.println("failed to mount FS");
	}
	//end read
}


void writeConfig() {

	//clean FS, for testing
	//SPIFFS.format();

	Serial.println("\nsaving config");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["http_server"] = shield.getServerName();
	json["http_port"] = shield.getServerPort();
	json["mqtt_server"] = shield.getMQTTServer();
	json["mqtt_port"] = shield.getMQTTPort();
	//json["mqtt_user"] = shield.getMQTTUser();
	//json["mqtt_password"] = shield.getMQTTPasssword();
	//json["resetsettings"] = shield.getResetSettings();
	//json["shieldid"] = shield.getShieldId();

	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		Serial.println("failed to open config file for writing");
		return;
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	Serial.println("config file writtten");
	//end save
}




void setup()
{
	Serial.begin(115200);
	delay(10);

	lastReboot = millis();
	shield.lastCheckHealth = millis();

	Logger::init();
	logger.print(tag, F("\n\t >>setup"));
	logger.print(tag, F("\n\n *******************RESTARTING************************"));

	shield.init();

	//writeConfig();

	readConfig();



#ifdef ESP8266
	shield.drawString(0, 0, F("restarting.."), 1, ST7735_WHITE);
#endif
	shield.setEvent(F("restarting.."));
	shield.invalidateDisplay();

	// Initialising the UI will init the display too.
	logger.print(tag, F("\n\t Versione="));
	//String str = ;
	logger.print(tag, shield.swVersion);

	// get MAC Address
	logger.print(tag, F("\n\tMAC Address="));
	WiFi.macAddress(shield.MAC_array);
	for (int i = 0; i < sizeof(shield.MAC_array); ++i) {
		if (i > 0) sprintf(shield.MAC_char, "%s:", shield.MAC_char);
		sprintf(shield.MAC_char, "%s%02x", shield.MAC_char, shield.MAC_array[i]);

	}
	logger.print(tag, shield.MAC_char);
#ifdef ESP8266
	//shield.drawString(0, 20, F("read eprom.."), 1, ST7735_WHITE);
#endif
	shield.setEvent(F("read eprom.."));
	shield.invalidateDisplay();

	//initEPROM();

	// disabilita il watchdog sw e abilita quello hw
#ifdef ESP8266
	ESP.wdtDisable();
#endif

	// Connect to WiFi network
	if (testWifi()) {

		shield.setEvent(F("connecting wifi.."));
		shield.invalidateDisplay();

		ESP.wdtFeed();
		//checkHTTPUpdate = false;
		checkForSWUpdate();

		shield.localIP = WiFi.localIP().toString();
		logger.print(tag, shield.localIP);

		mqttLoaded = false;

		shield.setEvent(F("Init MQTT"));
		initMQTTServer();
		mqttLoaded = true;

		//readConfig();

		//shield.setEvent(F("Init MQTT"));
		//logger.print(tag, "\n\n\tINIT MQTT");
		//initMQTTServer();
		//mqttLoaded = true;
		//requestSettingsFromServer();
	}

	logger.println(tag, F("\n\t<<setup\n\n"));
}

bool mqtt_publish(String topic, String message) {

	logger.print(tag, F("\n\t >>mqtt_publish"));

	logger.print(tag, F("\n\t topic: "));
	logger.print(tag, topic);
	logger.print(tag, F("\n\t message: "));
	logger.print(tag, message);

	bool res = false;
	if (!client.connected()) {
		logger.print(tag, F("\n\t OFFLINE - payload NOT sent!!!\n"));
	}
	else {
		res = mqttclient.publish(topic.c_str(), message.c_str());
		if (!res) {
			lastCommandFailed = millis();
		}
	}
	logger.print(tag, F("\n\t <<mqtt_publish res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

void checkForSWUpdate() {

#ifdef ESP8266

	logger.println(tag, F(">>checkForSWUpdate"));
	//delay(2000);

	//t_httpUpdate_return ret = ESPhttpUpdate.update("http://192.168.1.3:8080//webduino/ota",Shield::swVersion);
	String updatePath = "http://" + shield.getServerName() + ":" + shield.getServerPort() + "//webduino/ota";
	logger.print(tag, "\n\tcheck for sw update " + updatePath);
	logger.print(tag, "\n\tcurrent version " + shield.swVersion);
	t_httpUpdate_return ret = ESPhttpUpdate.update(updatePath, shield.swVersion);

	switch (ret) {
	case HTTP_UPDATE_FAILED:

		logger.print(tag, F("\n\tHTTP_UPDATE_FAILD Error "));
		logger.print(tag, String(ESPhttpUpdate.getLastError()));
		logger.print(tag, F(" "));
		logger.print(tag, ESPhttpUpdate.getLastErrorString().c_str());

		shield.setEvent(F("sw Update failed"));
		break;

	case HTTP_UPDATE_NO_UPDATES:
		logger.print(tag, F("\n\tHTTP_UPDATE_NO_UPDATES"));
		shield.setEvent(F("no sw update available"));
		break;

	case HTTP_UPDATE_OK:
		logger.print(tag, F("\n\tHTTP_UPDATE_OK"));
		shield.setEvent(F("sw updated"));
		break;
	}
	logger.println(tag, F("<<checkForSWUpdate"));

#endif
}

void initMQTTServer() {
	logger.print(tag, F("\n"));
	logger.println(tag, F(">>initMQTTServer"));
	mqttclient.init(&client);
	mqttclient.setServer(shield.getMQTTServer(), shield.getMQTTPort());
	mqttclient.setCallback(messageReceived);
	reconnect();
	logger.println(tag, F("<<initMQTTServer\n"));
}

void loop()
{
#ifdef ESP8266
	ESP.wdtFeed();

	/*if (checkHTTPUpdate) {
		shield.setEvent(F("check sw update"));
#ifdef ESP8266
		ESP.wdtFeed();
#endif
		checkHTTPUpdate = false;
		checkForSWUpdate();
	}*/

	/*if (!mqttLoaded) {
		shield.setEvent(F("Init MQTT"));
		initMQTTServer();
		mqttLoaded = true;
		return;
	}*/


#endif // ESP8266
	shield.setFreeMem(ESP.getFreeHeap());
	if (ESP.getFreeHeap() < 2000) {
		logger.println(tag, F("\n\n\n\LOW MEMORY"));
		logger.printFreeMem(tag, "");
		ESP.restart();
	}

	unsigned long currMillis = millis();
	if (currMillis - lastReboot > reboot_interval) {
		shield.setEvent(F("timeout reboot"));
		logger.println(tag, F("\n\n\n\-----------lastReboot TIMEOUT REBOOT--------\n\n"));
		ESP.restart();
	}

	currMillis = millis();
	if (currMillis - shield.lastCheckHealth > shield.checkHealth_timeout) {
		shield.setEvent(F("check health reboot"));
		logger.println(tag, F("\n\n\n\-----------CHECK HEALTH TIMEOUT REBOOT--------\n\n"));
		ESP.restart();
	}



#ifdef ESP8266
	wdt_enable(WDTO_8S);
#endif

	// controlla se l'ora è aggiornata (prima di richiedere i setting e dopo aver inizializzato mqtt)
	currMillis = millis();
	unsigned long timediff = currMillis - shield.lastTimeSync;
	if (client.connected() && (timediff > timeSync_interval || !shield.timeLoaded)) {
		shield.setEvent(F("Request time"));
		logger.print(tag, F("\n\n\t UPDATE TIME TIMEOUT"));
		shield.updateTime();
		shield.timeLoaded = true;
		shield.lastTimeSync = currMillis - 60000; // questo serve per evitare che richieda l'ora prima di averla ricevuta
		return;
	}

	if (client.connected() && !shield.settingFromServerRequested) {
		bool res = requestSettingsFromServer();
		if (!res) {
			logger.println(tag, F("\n request setting failed"));
			shield.readSensorFromFile();

			//ESP.restart();
		}
		return;
	}

	if (shield.settingFromServerRequested && !shield.settingFromServerReceived && millis() - shield.settingRequestedTime > 60 * 1000) {
		logger.println(tag, F("\n setting request timeout expired"));
		ESP.restart();
	}

	shield.checkStatus();

	// questo serve per controllare se il client mqtt è connesso
	// deve essere fatto dopo check status altrimenti la scheda non si aggiorna
	if (!client.connected()) {
		logger.println(tag, F("\n\n\tSERVER DISCONNECTED!!!\n"));
		shield.setEvent(F("server disconnected"));
		reconnect();
		delay(5000);
	}
	else {
		mqttclient.loop();
	}

	currMillis = millis();
	if (lastCommandFailed > 0 && currMillis - lastCommandFailed > commandFailed_interval) {
		logger.println(tag, F("\n\n\n\-----------lastCommandFailed TIMEOUT REBOOT--------\n\n"));
		ESP.restart();
	}
}
