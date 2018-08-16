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
#include "ESP8266Webduino.h"
#include "MQTTClientClass.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "Logger.h"
#include "Shield.h"
#include "Command.h"
#include <Time.h>
#include "TimeLib.h"
#include "POSTData.h"
#include <Wire.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "Ticker.h"
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

//void triggerUpdateTime();

/*Ticker updatetimeTimer(triggerUpdateTime, 30000); // once, immediately
bool timeNotUpdated = true;

void triggerUpdateTime() {
	logger.println(tag, F("\n\t >triggerUpdateTime"));
	timeNotUpdated = true;
}*/
extern void resetWiFiManagerSettings();
extern bool mqtt_publish(String topic, String message);
extern void reboot(String reason);


bool checkHTTPUpdate = true; //true;
bool mqttLoaded = false; //true;
bool shouldSaveConfig = true;
WiFiManager wifiManager;
ESPDisplay espDisplay;
Logger logger;
String tag = "Webduino";
Shield shield;
MQTTClientClass mqttclient;

WiFiServer server(80);
WiFiClient client;

unsigned long lastCommandFailed = 0;
const int commandFailed_interval = 60000 * 30;// *12;// 60 secondi * 15 minuti
unsigned long lastReboot = 0;
const int reboot_interval = 3600000 * 24;// 24 ore

extern int __bss_end;
extern void *__brkval;

void resetWiFiManagerSettings() {
	logger.print(tag, F("\n\n\t >>resetWiFiManagerSettings"));
	wifiManager.resetSettings();
	logger.println(tag, F("\n\t <<resetWiFiManagerSettings"));
}

void reboot(String reason) {
	shield.setRebootReason(reason);
	writeRebootReason();
	ESP.restart();
}

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
		//ESP.restart();
		reboot("Autoconnect Timeout");

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

	logger.println(tag, F(">>messageReceived"));
	logger.print(tag, F("\n\t topic="));
	logger.print(tag, String(topic));

#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266

	String message = "";
	for (int i = 0; i < length; i++) {
		message += char(payload[i]);
	}
	shield.parseMessageReceived(String(topic), message);
	logger.println(tag, F("<<messageReceived"));
}

bool reconnect() {
	logger.print(tag, F("\n\n\t >>reconnect"));
	shield.setStatus(F("CONNECTING.."));
	// Loop until we're reconnected
	String clientId = "ESP8266Client" + shield.getMACAddress();
	int i = 0;
	while (!mqttclient.connected() && i < 3) {
		ESP.wdtFeed();
		logger.print(tag, F("\n\t Attempting MQTT connection..."));
		logger.print(tag, F("\n\t temptative "));
		logger.print(tag, String(i));
		// Attempt to connect			
		if (mqttclient.connect(clientId)) {
			logger.print(tag, F("\n\t connected"));
			// Once connected, publish an announcement...
			//mqttclient.publish("send"/*topic.c_str()*/, "hello world");
			String topic = "fromServer/shield/" + shield.getMACAddress() + "/#";
			logger.print(tag, F("\n\t Subscribe to topic:"));
			logger.print(tag, topic);
			mqttclient.subscribe(topic.c_str());
			shield.setStatus(F("ONLINE"));
			logger.print(tag, F("\n\t <<reconnect\n\n"));
			return true;
		}
		else {
			logger.print(tag, F("\n\tfailed, rc="));
			logger.print(tag, mqttclient.state());
			logger.print(tag, F("\n\ttry again in 1 seconds"));
			// Wait 1 seconds before retrying
			delay(1000);
			i++;
		}
	}

	shield.setStatus(F("OFFLINE"));
	logger.print(tag, F("\n\t<<reconnect FAILED\n\n"));
	return false;
}

void readConfig() {

	logger.println(tag, F(">>readConfig"));
	
	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					logger.print(tag, F("\n\t parsed json"));

					if (json.containsKey("http_server")) {
						logger.print(tag, F("\n\t http_server: "));
						String str = json["http_server"];
						logger.print(tag, str);
						shield.setServerName(str);
					}
					if (json.containsKey("http_port")) {
						Serial.println("http_port: ");
						String str = json["http_port"];
						logger.print(tag,str);
						shield.setServerPort(json["http_port"]);
					}
					if (json.containsKey("mqtt_server")) {
						Serial.println("mqtt_server: ");
						String str = json["mqtt_server"];
						logger.print(tag, str);
						shield.setMQTTServer(json["mqtt_server"]);
					}
					if (json.containsKey("mqtt_port")) {
						Serial.println("mqtt_port: ");
						String str = json["mqtt_port"];
						logger.print(tag, str);
						shield.setMQTTPort(json["mqtt_port"]);
					}
					if (json.containsKey("mqtt_user")) {
						Serial.println("mqtt_user: ");
						String str = json["mqtt_user"];
						logger.print(tag, str);
						shield.setMQTTUser(json["mqtt_user"]);
					}
					if (json.containsKey("mqtt_password")) {
						Serial.println("mqtt_password: ");
						String str = json["mqtt_password"];
						logger.print(tag, str);
						shield.setMQTTPassword(json["mqtt_password"]);
					}
					if (json.containsKey("shieldid")) {
						Serial.println("shieldid: ");
						String str = json["shieldid"];
						logger.print(tag, str);
						shield.setShieldId(json["shieldid"]);
					}
					// resetsettings
					if (json.containsKey("resetsettings")) {
						Serial.println("resetsettings: ");
						String str = json["resetsettings"];
						logger.print(tag, str);
						//shield.setResetSettings(json["resetsettings"]);
					}					
				}
				else {
					logger.print(tag, F("\n\t failed to load json config"));
					//clean FS, for testing
					SPIFFS.format();
					writeConfig();
				}
			}
		}
	}
	else {
		logger.print(tag, F("failed to mount FS"));
	}
	logger.print(tag, F("<<readConfig"));
}

void writeConfig() {

	logger.println(tag, F(">>writeConfig"));
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["http_server"] = shield.getServerName();
	json["http_port"] = shield.getServerPort();
	json["mqtt_server"] = shield.getMQTTServer();
	json["mqtt_port"] = shield.getMQTTPort();
	json["mqtt_user"] = shield.getMQTTUser();
	json["mqtt_password"] = shield.getMQTTPassword();
	json["resetsettings"] = shield.getResetSettings();
	json["shieldid"] = shield.getShieldId();

	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		logger.print(tag, F("<<writeConfig"));
		return;
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	logger.println(tag, F("config file writtten"));
	logger.println(tag, F("<<writeConfig"));
}

void writeRebootReason() {

	logger.println(tag, " >>writeRebootReason config");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["rebootreason"] = shield.getRebootReason();

	File configFile = SPIFFS.open("/reason.json", "w");
	if (!configFile) {
		logger.print(tag, "\n\t failed to open rebootreason file for writing");
		return;
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	logger.print(tag, "\n\t reason file writtten");
	logger.println(tag, "writeRebootReason config");
	//end save
}

void readRebootReason() {

	logger.println(tag, F(">>readRebootReason"));

	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/reason.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					logger.print(tag, F("\n\t parsed json"));

					
					if (json.containsKey("rebootreason")) {
						Serial.println("rebootreason: ");
						String str = json["rebootreason"];
						logger.print(tag, str);
						String reason = json["rebootreason"];
						shield.setRebootReason(str);
					}

				}
				else {
					logger.print(tag, F("\n\t failed to load json config"));
					//clean FS, for testing
					SPIFFS.format();
					writeConfig();
				}
			}
		}
	}
	else {
		logger.print(tag, F("failed to mount FS"));
	}
	logger.print(tag, F("<<readRebootReason"));
}

void setup()
{
	Serial.begin(115200);
	delay(10);


	Logger::init();
	logger.print(tag, F("\n\t >>setup"));
	logger.print(tag, F("\n\n *******************RESTARTING************************"));

	shield.init();

	shield.setEvent(F("read config.."));
	shield.invalidateDisplay();
	readConfig();
	readRebootReason();
#ifdef ESP8266
	shield.drawString(0, 0, F("restarting.."), 1, ST7735_WHITE);
#endif
	shield.setEvent("restarting...ver" + shield.swVersion);
	shield.invalidateDisplay();
	logger.print(tag, F("\n\t Versione="));
	logger.print(tag, shield.swVersion);

	// get MAC Address
	/*logger.print(tag, F("\n\tMAC Address="));
	WiFi.macAddress(shield.MAC_array);
	for (int i = 0; i < sizeof(shield.MAC_array); ++i) {
		if (i > 0) sprintf(shield.MAC_char, "%s:", shield.MAC_char);
		sprintf(shield.MAC_char, "%s%02x", shield.MAC_char, shield.MAC_array[i]);

	}
	logger.print(tag, shield.MAC_char);*/

	// Connect to WiFi network
	if (testWifi()) {

		shield.setEvent(F("connecting wifi.."));
		shield.invalidateDisplay();

		checkForSWUpdate();

#ifdef ESP8266
		ESP.wdtDisable();
#endif
		ESP.wdtFeed();

		shield.localIP = WiFi.localIP().toString();
		logger.print(tag, shield.localIP);

		shield.setEvent(F("Init MQTT"));
		mqttLoaded = initMQTTServer();
		//mqttLoaded = true;
	}

	lastReboot = millis();
	shield.lastCheckHealth = millis();

	logger.println(tag, F("\n\t<<setup\n\n"));
}

bool mqtt_publish(String topic, String message) {

	logger.print(tag, F("\n\t >>mqtt_publish"));
	logger.print(tag, F("\n\t topic: "));
	logger.print(tag, topic);
	logger.print(tag, F("\n\t message: "));
	logger.print(tag, message);

#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266

	bool res = false;
	if (!client.connected()) {
		logger.print(tag, F("\n\t OFFLINE - payload NOT sent!!!\n"));
	}
	else {
		res = mqttclient.publish(topic.c_str(), message.c_str());
		if (!res) {
			logger.print(tag, F("\n\t lastcommandfailed"));
			//lastCommandFailed = millis();
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

bool initMQTTServer() {
	logger.print(tag, F("\n"));
	logger.println(tag, F(">>initMQTTServer"));
	mqttclient.init(&client);
	mqttclient.setServer(shield.getMQTTServer(), shield.getMQTTPort());
	mqttclient.setCallback(messageReceived);
	bool res = reconnect();
	logger.println(tag, F("<<initMQTTServer ="));
	logger.println(tag, Logger::boolToString(res));
}

void loop()
{
	//logger.print(tag, "\n\t OFFLIN");
#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266
	shield.setFreeMem(ESP.getFreeHeap());

	unsigned long currMillis = millis();
	if (currMillis - lastReboot > reboot_interval) {
		shield.setEvent(F("timeout reboot"));
		logger.println(tag, F("\n\n\n\-----------lastReboot TIMEOUT REBOOT--------\n\n"));
		reboot("daily reboot");
	}

	currMillis = millis();
	if (currMillis - shield.lastCheckHealth > shield.checkHealth_timeout) {
		shield.setEvent(F("check health reboot"));
		logger.println(tag, F("\n\n\n\-----------CHECK HEALTH TIMEOUT REBOOT--------\n\n"));
		//ESP.restart();
		reboot("Check health Timeout");
	}

#ifdef ESP8266
	wdt_enable(WDTO_8S);
#endif

	shield.checkStatus();

	if (client.connected()) {
		
		//currMillis = millis();
		
		shield.checkTimeUpdateStatus();

		shield.checkSettingResquestStatus();

		

		mqttclient.loop();
	}
	else {
		logger.println(tag, F("\n\n\tSERVER DISCONNECTED!!!\n"));
		shield.setEvent(F("server disconnected"));
		reconnect();
		delay(5000);
	}

	/*if (shield.settingFromServerRequested && !shield.settingFromServerReceived && millis() - shield.settingRequestedTime > 60 * 1000) {
		logger.println(tag, F("\n setting request timeout expired"));
		ESP.restart();
	}*/



	// questo serve per controllare se il client mqtt è connesso
	// deve essere fatto dopo check status altrimenti la scheda non si aggiorna
	/*if (!mqttLoaded) {
		logger.println(tag, F("\n\n\t MQTT NOT LOADED!!!\n"));
		shield.setEvent(F("mqtt not loaded"));
		mqttLoaded = initMQTTServer();
		delay(5000);
	}*/


	/*currMillis = millis();
	if (lastCommandFailed > 0 && currMillis - lastCommandFailed > commandFailed_interval) {
		logger.println(tag, F("\n\n\n\-----------lastCommandFailed TIMEOUT REBOOT--------\n\n"));
		ESP.restart();
	}*/
}
