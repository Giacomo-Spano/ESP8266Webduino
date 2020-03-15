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
#include "MQTTMessage.h"
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

#include "SoftwareSerial.h"
#endif


//SoftwareSerial swSer1(D1, D2, false, 256);
//extern SoftwareSerial* swSer1;
//extern SoftwareSerial swSer1(D1, D2, false, 256);

//extern SoftwareSerial *swSer1;
//extern SoftwareSerial nextionSoftwareSerial;


#ifdef ESP8266
//#include "NextionDisplay.h"
//NextionDisplay nextDisplay;
#endif

//void triggerUpdateTime();

/*Ticker updatetimeTimer(triggerUpdateTime, 30000); // once, immediately
bool timeNotUpdated = true;

void triggerUpdateTime() {
	logger.println(tag, F("\n\t >triggerUpdateTime"));
	timeNotUpdated = true;
}*/
extern void resetWiFiManagerSettings();
extern bool mqtt_publish(MQTTMessage mqttmessage);
extern bool mqtt_subscribe(String topic);
extern void messageReceived(char* topic, byte* payload, unsigned int length);
//extern void messageReceived(String topic, String message);
extern void reboot(String reason);


bool shouldSaveConfig = true;
WiFiManager wifiManager;
Logger logger;
String tag = "Webduino";
Shield shield;
MQTTClientClass mqttclient;
LinkedList<MQTTMessage*> mqttmessagelist = LinkedList<MQTTMessage*>();

WiFiClient client;

unsigned long lastReboot = 0;
const int reboot_interval = 86400000;// 24 ore

void resetWiFiManagerSettings() {
	logger.print(tag, F("\n\n\t >>resetWiFiManagerSettings"));
	wifiManager.resetSettings();
	logger.println(tag, F("\n\t <<resetWiFiManagerSettings"));
}

void reboot(String reason) {
	shield.setRebootReason(reason);
	shield.writeRebootReason();
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
	//bool oleddisplay = String(shield.getOledDisplay());
	
	WiFiManagerParameter custom_server("server", "server", server.c_str(), 40);
	WiFiManagerParameter custom_server_port("port", "port", serverport.c_str(), 5);
	WiFiManagerParameter custom_mqtt_server("mqttserver", "mqtt server", mqttserver.c_str(), 40);
	WiFiManagerParameter custom_mqtt_port("mqttport", "mqtt port", mqttport.c_str(), 5);
	
	/*char customhtml[24] = "type=\"checkbox\"";
	if (oleddisplay) {
		strcat(customhtml, " checked");
	}
	WiFiManagerParameter p_sensorDht22("sensordht22", "DHT-22 Sensor", "T", 2, customhtml, WFM_LABEL_AFTER);*/


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
	//wifiManager.resetSettings();
	/*WiFi.persistent(true);
	WiFi.disconnect(true);
	WiFi.persistent(false);
	wifiManager.resetSettings();
	wifiManager.setBreakAfterConfig(true);*/

	logger.println(tag, F("before autoconnect..."));
	//wifiManager.setConfigPortalBlocking(false);

	if (!wifiManager.autoConnect("WifiSetup")) {
		Serial.println("failed to connect, we should reset as see if it connects");
		delay(3000);
		ESP.restart();
		delay(5000);
	}
	else {
		Serial.println("Configportal running");
	}
	logger.println(tag, F("after autoconnect..."));




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

	logger.println(tag, F("trying to connect..."));
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

		shield.setResetSettings(false);

		shield.writeConfig();
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
	//messageReceived(String(topic), message);
	logger.println(tag, F("<<messageReceived"));
}

/*void messageReceived(String topic, String message) {

	logger.print(tag, F("\n\t **>>messageReceived"));
	logger.print(tag, F("\n\t **topic="));
	logger.print(tag, topic);
	logger.print(tag, F("\n\t **message="));
	logger.print(tag, message);

	shield.parseMessageReceived(topic, message);
	logger.println(tag, F("\n\t **<<messageReceived"));
}*/


bool reconnect() {
	logger.print(tag, F("\n\n\t >>reconnect"));
	shield.setStatus(F("CONNECTING.."));
	// Loop until we're reconnected
	String clientId = "ESP8266Client" + shield.getMACAddress();
	int i = 0;
	while (!mqttclient.connected() && i < 3) {
#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266
		logger.print(tag, F("\n\t Attempting MQTT connection..."));
		logger.print(tag, F("\n\t temptative "));
		logger.print(tag, String(i));
		// Attempt to connect			
		if (mqttclient.connect(clientId,shield.getMQTTUser(),shield.getMQTTPassword())) {
			logger.print(tag, F("\n\t connected"));
			// Once connected, publish an announcement...
			String topic = "fromServer/shield/" + shield.getMACAddress() + "/#";
			mqttclient.subscribe(topic.c_str());

			if (shield.getLoRaGatewayServer()) {
				String topic = "fromServer/shield/" + shield.getLoRaGatewayTargetAddress() + "/#";
				logger.print(tag, F("\n\t Subscribe to topic:"));
				logger.print(tag, topic);
				mqttclient.subscribe(topic.c_str());
			}
			/*if (shield.getLoRaGatewayServer()) {
				String topic2 = "fromServer/shield/" + shield.getLoRaGatewayTargetAddress() + "/#";
				logger.print(tag, F("\n\t Subscribe to topic:"));
				logger.print(tag, topic);
				
				mqttclient.subscribe(topic2.c_str());
			}*/

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

//#ifndef ESP8266
//#include <FS.h> //this needs to be first, or it all crashes and burns...
//#include "SPIFFS.h"
//#endif
//#include "LittleFS.h"
#include "FS.h"

void listAllFiles() {

	
	/*File f = SPIFFS.open("/", "r");
	if (!f) {
		Serial.println("file open failed");
	}*/

	logger.print(tag, F("\n\n listAllFiles"));
	logger.print(tag, F("\n\tname \t\tsize"));
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		//Serial.print(dir.fileName());
		logger.print(tag, F("\n\t"));
		logger.print(tag, dir.fileName());
		File f = dir.openFile("r");
		logger.print(tag, F("\t"));
		logger.print(tag, f.size());
		//Serial.println(f.size());

		logger.print(tag, F("\n"));
		String data = f.readString();
		Serial.println(data);
		f.close();
		logger.print(tag, F("\n"));

	}
	logger.print(tag, F("\n"));


	//File file = root.openNextFile();

	/*while (file) {

		Serial.print("FILE: ");
		Serial.println(file.name());

		file = root.openNextFile();
	}*/

}

void setup()
{
#ifdef TTGO
	pinMode(16, OUTPUT);
	pinMode(2, OUTPUT);

	digitalWrite(16, LOW); // set GPIO16 low to reset OLED
	delay(50);
	digitalWrite(16, HIGH); // while OLED is running, GPIO16 must go high
	
#endif // TTGO
	//Wire.begin(D4,D3/*sdaPin, sclPin*/);

	//Serial.begin(9600);
	Serial.begin(115200);
	delay(10);

	Logger::init();
	//logger.print(tag, F("\n\t >>setup"));
	logger.print(tag, F("\n\n *******************RESTARTING************************"));

	////////
	
	Serial.println(F("Inizializing FS..."));
	if (SPIFFS.begin()) {
		Serial.println(F("done."));
	}
	else {
		Serial.println(F("fail."));
	}
	if (!SPIFFS.begin()) {
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}

	Serial.println("\n\n----Listing files before format----");
	listAllFiles();

	/*bool formatted = SPIFFS.format();

	if (formatted) {
		Serial.println("\n\nSuccess formatting");
	}
	else {
		Serial.println("\n\nError formatting");
	}*/

	////////////////




	shield.init();

#ifdef ESP8266
//	nextDisplay.init();
#endif

	shield.setEvent(F("read config.."));
	shield.invalidateDisplay();
	shield.readConfig();
	shield.readRebootReason();
#ifdef ESP8266
	shield.drawString(0, 0, F("restarting.."), 1, ST7735_WHITE);
#endif
	shield.setEvent("restarting...ver" + shield.swVersion);
	shield.invalidateDisplay();
	logger.print(tag, F("\n\t Versione="));
	logger.print(tag, shield.swVersion);
		
	// Connect to WiFi network
	if (shield.getLoRaGateway() && !shield.getLoRaGatewayServer()) {
		shield.setEvent(F("lora gateway client"));
		shield.readSensorFromFile();
	} else if (testWifi()) {

		shield.setEvent(F("connecting wifi.."));
		shield.invalidateDisplay();

		checkForSWUpdate();

#ifdef ESP8266
		ESP.wdtDisable();
		ESP.wdtFeed();
#endif
		shield.localIP = WiFi.localIP().toString();
		logger.print(tag, shield.localIP);

		shield.setEvent(F("Init MQTT"));
		/*mqttLoaded = */initMQTTServer();
	}

	lastReboot = millis();
	shield.lastCheckHealth = millis();

	logger.println(tag, F("\n\t<<setup\n\n"));
}

bool mqtt_subscribe(String topic) {
	
	logger.print(tag, F("\n\t mqtt_subscribe"));
	if (mqttclient.connected())
		mqttclient.subscribe(topic.c_str());
}

bool mqtt_publish(MQTTMessage mqttmessage) {

	logger.print(tag, String(F("\n\t >>mqtt_publish \n\t topic:")) + mqttmessage.topic);
	logger.print(tag, String(F("\n\t message:")) + mqttmessage.message);

	//logger.print(tag, String(F("\n\t shield.getLoRaGateway():")) + Logger::boolToString(shield.getLoRaGateway()));
	//logger.print(tag, String(F("\n\t shield.getLoRaGatewayServer():")) + Logger::boolToString(shield.getLoRaGatewayServer()));

	if (shield.getLoRaGateway() && !shield.getLoRaGatewayServer()) {		
		// se è un client lora manda un messaggio lora la server
		// invece di pubblicare il messaggio MQQT
		
		logger.print(tag, "\n\n\t ----LORA CLIENT ------");
		logger.print(tag, "\n\n\t >PUBISH LORA MESSAGE");

		String payload = String(mqttmessage.topic.length()) + ";" + mqttmessage.topic + ";" +
			String(mqttmessage.message.length()) + ";" + mqttmessage.message;
		logger.print(tag, "\n\t >>payload= " + payload);

		shield.sendLoRaMessage(payload);
		logger.print(tag, "\n\n\t > LORA MESSAGE PUBLISHED");
		return true;
	}

	mqttmessagelist.add(&mqttmessage);
	logger.print(tag, F("\n\t mqttmessagelist size:"));
	logger.print(tag, mqttmessagelist.size());

	if (mqttmessagelist.size() > 10) {
		reboot(F("too many message"));
	}

	for (int i = 0; i < mqttmessagelist.size(); i++)
	{
#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266

		MQTTMessage* message = (MQTTMessage*)mqttmessagelist.get(i);
		logger.print(tag, F("\n\t message:"));
		logger.print(tag, mqttmessage.message);

		//bool res = mqtt_publish(message->topic, message->message);
		bool res = false;
		if (!client.connected()) {
			logger.print(tag, F("\n\t OFFLINE - payload NOT sent!!!\n"));
		}
		else {
			res = mqttclient.publish(message->topic.c_str(), message->message.c_str());
			if (!res) {
				logger.print(tag, F("\n\t MQTT Message not sent!!!\n"));
				return false;
			} else {
				mqttmessagelist.remove(i);
				i--;
				logger.print(tag, F("\n\t MQTT Message sent!!!\n"));
				continue;
			}
		}
		
	}
	logger.print(tag, F("\n\t << mqtt_publish"));
	return true;
}

void checkForSWUpdate() {

#ifdef ESP8266

	logger.println(tag, F(">>checkForSWUpdate"));
	//delay(2000);

	String updatePath = "http://" + shield.getServerName() + ":" + shield.getServerPort() + "//webduino/ota";
	logger.print(tag, "\n\t check for sw update " + updatePath);
	logger.print(tag, "\n\t current version " + shield.swVersion);
	t_httpUpdate_return ret = ESPhttpUpdate.update(updatePath, shield.swVersion);

	switch (ret) {
	case HTTP_UPDATE_FAILED:

		logger.print(tag, F("\n\t HTTP_UPDATE_FAILD Error "));
		logger.print(tag, String(ESPhttpUpdate.getLastError()));
		logger.print(tag, F(" "));
		logger.print(tag, ESPhttpUpdate.getLastErrorString().c_str());

		shield.setEvent(F("sw Update failed"));
		break;

	case HTTP_UPDATE_NO_UPDATES:
		logger.print(tag, F("\n\t HTTP_UPDATE_NO_UPDATES"));
		shield.setEvent(F("no sw update available"));
		break;

	case HTTP_UPDATE_OK:
		logger.print(tag, F("\n\t HTTP_UPDATE_OK"));
		shield.setEvent(F("sw updated"));
		break;
	}
	logger.println(tag, F("<<checkForSWUpdate"));

#endif
}

bool initMQTTServer() {
	logger.println(tag, F("\n\t >>initMQTTServer"));
	mqttclient.init(&client);
	mqttclient.setServer(shield.getMQTTServer(), shield.getMQTTPort());
	mqttclient.setCallback(messageReceived);
	bool res = reconnect();
	logger.println(tag, String(F("\n\t <<initMQTTServer =")) + Logger::boolToString(res));
}

void loop()
{	
	//logger.println(tag, F("\nloop"));
	//nexLoop(nex_listen_list);
#ifdef NEXTDISPLAY

	nextDisplay.loop();

#endif

#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266
	shield.setFreeMem(ESP.getFreeHeap());

	unsigned long currMillis = millis();
	if (currMillis - lastReboot > reboot_interval) {
		shield.setEvent(F("timeout reboot"));
		logger.println(tag, F("\n\n\n\-----------lastReboot TIMEOUT REBOOT--------\n\n"));
		reboot("daily reboot");
		lastReboot = currMillis;
	}

	currMillis = millis();
	if (currMillis - shield.lastCheckHealth > shield.checkHealth_timeout) {
		shield.setEvent(F("check health reboot"));
		logger.println(tag, F("\n\n\n\-----------CHECK HEALTH TIMEOUT REBOOT--------\n\n"));
		//reboot("Check health Timeout");
		shield.lastCheckHealth = currMillis;
	}

#ifdef ESP8266
	wdt_enable(WDTO_8S);
#endif

	shield.checkStatus();

	if (client.connected()) {
		shield.checkTimeUpdateStatus();
		shield.checkSettingResquestStatus();
		mqttclient.loop();
	}
	else {
		if (shield.getLoRaGateway() && !shield.getLoRaGatewayServer()) {

		} else {
			logger.println(tag, F("\n\n\tSERVER DISCONNECTED!!!\n"));
			shield.setEvent(F("server disconnected"));
			reconnect();
			delay(5000);
		}
	}
}
