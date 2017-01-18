#include <OneWire.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "user_interface.h"
#include "wol.h"
#include "Logger.h"
#include "HttpHelper.h"
#include "JSON.h"
//#include "OneWireSensors.h"
#include "Shield.h"
#include "Command.h"
#include "Program.h"
#include "Actuator.h"
#include "DS18S20Sensor.h"
#include <Time.h>
#include "TimeLib.h"
#include "css.h"
#include "FS.h"

extern uint8_t OneWirePin = D4;
extern OneWire oneWire(OneWirePin);
extern DallasTemperature sensors(&oneWire);

#define Versione 0.92
//int buildVersion = 1;

int EPROM_Table_Schema_Version = 5;
const char SWVERSION[] = "1.01";

Logger logger;
String sensorNames = "";

const char* ssidx = "Telecom-29545833";
const char* password = "6oGzjkdMJU2q9XoQLfWiV3nj";

String tag = "Webduino";
const char *ssidAP = "ES8266";
const char *passwordAP = "thereisnospoon";
const char* ssid = "xxBUBBLES";
/*   da eliminare*/
byte mac[] = { 0x00, 0xA0, 0xB0, 0xC0, 0xD0, 0x90 };

extern const char* statusStr[] = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };
Shield shield;


void initEPROM();
void readEPROM();
void writeEPROM();
void showPage(String data);
void getPostdata(char *data, int maxposdata);
int parsePostdata(const char* data, const char* param, char* value);
String getJsonStatus();
String getJsonSensorsStatus();
String getJsonActuatorsStatus();
int findIndex(const char* data, const char* target);
String showMain(String param);
String showSettings(String param);
String showIODevices(String param);
void showChangeIODevices(String param);
void showChangeSettings(String param);
void showPower(String param);
String softwareReset();
String showIndex();
void setRemoteTemperature(String param);
String getFile(String filename);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;


const int maxposdataChangeSetting = 150;
char databuff[maxposdataChangeSetting];

#define P(name) static const prog_uchar name[] PROGMEM // declare a static string
const byte EEPROM_ID = 0x99; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int TIMERTIME_ADDR = 1; // the EEPROM address used to store the pin

const int maxposdata = 101; // massimo numero di caratteri della variabile posdata
const int maxvaluesize = maxposdata - 1/*10*/; // massimo numero di digit del valore di una variabile nel POS data

bool statusChangeSent = true;

const int flash_interval = 30000;
unsigned long lastFlash = 0;//-flash_interval;
const int lastStatusChange_interval = 60000; // timeout retry invio status change
unsigned long lastStatusChange = 0;//-lastStatusChange_interval;// 0;
unsigned long lastNotification = 0;
const int Notification_interval = 20000;

unsigned long lastSendLog = 0;
const int SendLog_interval = 10000;// 10 secondi

unsigned long lastTimeSync = 0;
const int timeSync_interval = 60000;// 60 secondi

int offlineCounter = 0;

// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

int sendRestartNotification = 0;
bool shieldRegistered = false; // la shield si registra all'inizio e tutte le volte che va offline

void writeEPROM() {

	logger.println(tag, "write EPROM");

	int addr = TIMERTIME_ADDR;
	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data
	byte hiByte;
	byte loByte;

	// dummy
	byte dummy;
	dummy = 3;
	EEPROM.write(addr++, dummy);

	// build version
	hiByte = highByte(EPROM_Table_Schema_Version);
	loByte = lowByte(EPROM_Table_Schema_Version);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);

	for (int i = 0; i < 10; i++) {
		hiByte = highByte(Shield::getIODevice(i));
		loByte = lowByte(Shield::getIODevice(i));
		EEPROM.write(addr++, hiByte);
		EEPROM.write(addr++, loByte);
	}
	
	// local port
	hiByte = highByte(shield.localPort);
	loByte = lowByte(shield.localPort);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	// ssid
	//Serial.print("networkSSID=");
	//Serial.println(shield.networkSSID);
	int res = EEPROM_writeAnything(addr, shield.networkSSID);
	addr += res;
	// password
	res = EEPROM_writeAnything(addr, shield.networkPassword);
	addr += res;
	// server name
	res = EEPROM_writeAnything(addr, shield.servername);
	addr += res;
	// server port
	hiByte = highByte(shield.serverPort);
	loByte = lowByte(shield.serverPort);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	// board name
	res = EEPROM_writeAnything(addr, shield.boardname);
	addr += res;
	// sensor names
	String str = "";
	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor* sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		str += String(sensor->sensorname);
		str += ";";
	}
	logger.print(tag, "\n\tstr=" + str);
	char buffer[100];
	str.toCharArray(buffer, sizeof(buffer));
	res = EEPROM_writeAnything(addr, buffer);
	addr += res;
	EEPROM.commit();
}

void readEPROM() {

	logger.println(tag, "read EPROM");

	byte hiByte;
	byte lowByte;
	int addr = TIMERTIME_ADDR;
	// dummy
	byte dummy = EEPROM.read(addr++);

	logger.print(tag, "\ndummy=");
	logger.print(tag, String(dummy));

	logger.print(tag, "\EPROM_Table_Schema_Version=");
	logger.print(tag, String(EPROM_Table_Schema_Version));

	int epromversion = 0;
	// build version
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	epromversion = word(hiByte, lowByte);
	logger.print(tag, "\nepromversion=");
	logger.print(tag, String(epromversion));
	
	if (epromversion >= EPROM_Table_Schema_Version) {
		for (int i = 0; i < Shield::getMaxIoDevices(); i++) {
			hiByte = EEPROM.read(addr++);
			lowByte = EEPROM.read(addr++);
			Shield::setIODevice(i,word(hiByte, lowByte));
			
			logger.print(tag, "\ni=");
			logger.print(tag, String(i));
			logger.print(tag, ", dev=");
			logger.print(tag, String(Shield::getIODevice(i)));
		}
	}

	// local port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	shield.localPort = word(hiByte, lowByte);
	// ssid
	int res = EEPROM_readAnything(addr, shield.networkSSID);
	//Serial.println("networkSSID=");
	//Serial.println(shield.networkSSID);
	addr += res;
	// password
	res = EEPROM_readAnything(addr, shield.networkPassword);
	//Serial.println("networkPassword=");
	//Serial.println(shield.networkPassword);
	addr += res;
	//server name
	res = EEPROM_readAnything(addr, shield.servername);
	addr += res;
	// server port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	shield.serverPort = word(hiByte, lowByte);
	// board name
	res = EEPROM_readAnything(addr, shield.boardname);
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
}

void initEPROM()
{
	logger.println(tag, "initEPROM");

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
}

int testWifi(void) {
	int c = 0;
	logger.println(tag, "Waiting for Wifi to connect");
	while (c < 20) {
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
	Serial.println("");
	Serial.println("");
	Serial.println("RESTARTING.... \n");

	logger.print(tag, "\n\n\"*******************RESTARTING************************--");


	// always use this to "mount" the filesystem
	bool result = SPIFFS.begin();
	Serial.println("SPIFFS opened: " + result);

	// this opens the file "f.txt" in read-mode
	File f = SPIFFS.open("/f.txt", "r");

	if (!f) {
		Serial.println("File doesn't exist yet. Creating it");

		// open the file in write mode
		File f = SPIFFS.open("/f.txt", "w");
		if (!f) {
			Serial.println("file creation failed");
		}
		// now write two lines in key/value style with  end-of-line characters
		f.println("ssid=abc");
		f.println("password=123455secret");
	}
	else {
		// we could open the file
		while (f.available()) {
			//Lets read line by line from the file
			String line = f.readStringUntil('\n');
			Serial.println(line);
		}

	}
	f.close();





	String str = "\n\nstarting.... Versione ";
	str += String(Versione);
	logger.print(tag, str);

	// get MAC Address
	logger.print(tag, "\n\tMAC Address=");
	WiFi.macAddress(shield.MAC_array);
	for (int i = 0; i < sizeof(shield.MAC_array); ++i) {
		if (i > 0) sprintf(shield.MAC_char, "%s:", shield.MAC_char);
		sprintf(shield.MAC_char, "%s%02x", shield.MAC_char, shield.MAC_array[i]);

	}
	logger.print(tag, shield.MAC_char);


	initEPROM();
	logger.print(tag, "\n\tSensorNames=" + sensorNames);

	// Connect to WiFi network
	logger.print(tag, "\n\nConnecting to " + String(shield.networkSSID) + " " + String(shield.networkPassword));

	WiFi.mode(WIFI_STA);//??????
	WiFi.begin(shield.networkSSID, shield.networkPassword);
	if (testWifi() == 20/*WL_CONNECTED*/) {

		checkOTA();

		// Start the server
		server.begin();
		logger.print(tag, "Server started");
		shield.localIP = WiFi.localIP().toString();
		// Print the IP address
		logger.println(tag, shield.localIP);

		wdt_disable();
		wdt_enable(WDTO_8S);

		// rele
		//pinMode(relePin, OUTPUT);
		//enableRele(false);
		shield.hearterActuator.init(String(shield.MAC_char));
		//Temperature sensor
		shield.addOneWireSensors(sensorNames);
		shield.addActuators();


		Command command;
		command.setServer(shield.servername, shield.serverPort);
		shield.id = command.registerShield(shield);

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
		// Print the IP address
		wdt_disable();
		wdt_enable(WDTO_8S);

		//programSettings.currentStatus = Program::STATUS_DISABLED;
		shield.hearterActuator.setStatus(Program::STATUS_DISABLED);
	}


}

void showwol(String param) {

	logger.println(tag, F("showwol "));

	/*wol* w = new wol();
	w->init();
	w->wakeup();
	delete w;*/

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>WOL</title></head><body>");
	data += F("</body></html>");
	client.println(data);
	client.stop();
}

String registerShield() {
	logger.println(tag, F("register shield "));
	
	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");
	data += F("REGISTER SHIELD - Attendere prego");
	data += F("</body></html>");

	//client.println(data);
	//client.stop();
	Command command;
	command.registerShield(shield);
	
	return data;
}

String softwareReset() {

	logger.println(tag, F("software reset "));



	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");
	data += F("SOFTWARE RESET - Attendere prego");
	data += F("</body></html>");

	client.println(data);
	client.stop();

	//delay(3000);
	//ESP.reset();
	ESP.restart();
	return "";
}

String getNextWord(int n, const char* source) {
	int len = strlen_P(html_index);
	String str = "";
	for (int k = 0; k < 1000; k++)
	{
		if (n + k >= len) {
			break;
		}
		char s = pgm_read_byte_near(source + n + k);
		str += s;
	}
	return str;
}

String showIndex() {

	logger.println(tag, F("showIndex"));

	String header = F("HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n");
	client.println(header);

	int len = strlen_P(html_index);
	int i = 0, k = 0;
	while (i < len) {
		String str = "";
		str = getNextWord(i, html_index);
		i += str.length();

		/*for (k = 0; k < 1000; k++)
		{
			if (i + k >= len) {
				break;
			}
			char s = pgm_read_byte_near(html_index + i + k);
			str += s;
		}
		i += k;*/


		String ip = WiFi.localIP().toString();
		str.replace("%datetime", logger.getStrDate());
		str.replace("%ipaddress", ip);
		str.replace("%macaddress", String(shield.MAC_char));
		str.replace("%shieldid", String(shield.id));
		if (shield.hearterActuator.getStatus() == Program::STATUS_DISABLED) {
			str.replace("%powerstatus", "OFF");
			//str.replace("%powervalue", "1");
			//str.replace("%powercommand", "ON");
		}
		else {
			str.replace("%powerstatus", "ON");
			//str.replace("%powervalue", "0");
			//str.replace("%powercommand", "OFF");
		}
		client.print(str);
	}

	return "";
}

String download() {

	logger.println(tag, F("download "));

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");
	data += F("SOFTWARE RESET - Attendere prego");
	data += F("</body></html>");

	client.println(data);

	Command command;
	command.download("ESP8266.css", shield);

	command.download("prova3.html", shield);
	//command.download("ESP8266.css", shield);


	client.stop();

	//delay(3000);
	//ESP.reset();

	return "";
}

String showRele(String GETparam) {

	logger.println(tag, F("\n\t>>called showRele "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];
	int status;
	// status
	status = parsePostdata(databuff, "status", posdata);
	logger.print(tag, F("\n\tstatus="));
	logger.print(tag, status);
	// duration
	long duration = parsePostdata(databuff, "duration", posdata);
	logger.print(tag, F("\n\tduration="));
	logger.print(tag, duration);
	logger.print(tag, F(" minuti"));
	duration = duration * 60 * 1000;
	logger.print(tag, F("\n\tduration ="));
	logger.print(tag, duration);
	logger.print(tag, F(" millisecondi"));
	// sensor
	String str = "";
	int sensorId = parsePostdata(databuff, "sensorid", posdata);
	str = String(sensorId);
	logger.print(tag, F("\n\tsensorId="));
	logger.print(tag, str);
	// target
	int res = parsePostdata(databuff, "target", posdata);
	//if (res != -1) {
	str = posdata;
	float target = str.toFloat();
	logger.print(tag, F("\n\ttarget ="));
	logger.print(tag, str);
	shield.hearterActuator.setTargetTemperature(target);
	
	// remote temperature
	float remoteTemperature = -1;
	res = parsePostdata(databuff, "temperature", posdata);
	if (res != -1) {
		str = "";
		str += posdata;
		remoteTemperature = str.toFloat();
		logger.print(tag, F("\n\tremoteTemperature ="));
		logger.print(tag, str);
		shield.hearterActuator.setRemoteTemperature(remoteTemperature);
	}
	else {
		logger.print(tag, F("\n\tremoteTemperature MISSING"));
	}
	// program
	int program = parsePostdata(databuff, "program", posdata);
	logger.print(tag, F("\n\tprogram="));
	logger.print(tag, program);
	// timerange
	int timerange = parsePostdata(databuff, "timerange", posdata);
	logger.print(tag, F("\n\ttimerange="));
	logger.print(tag, timerange);
	// local sensor
	int localSensor = parsePostdata(databuff, "localsensor", posdata);
	logger.print(tag, F("\n\tlocalsensor="));
	logger.print(tag, localSensor);
	
	// jsonRequest
	// DA CAMBIARE. Controllarer in base all'header
	int json = parsePostdata(databuff, "json", posdata);
	logger.print(tag, F("\n\tjson="));
	logger.print(tag, json);

	shield.hearterActuator.changeProgram(status,duration,
									!localSensor,
									remoteTemperature,
									sensorId,
									target, program, timerange);

	
	
	//lastFlash = millis() - flash_interval;

	//////////////////
	if (json == 1) {
		String data = getJsonStatus();
		client.println(data);
	}
	else {
		String data;
		data += "";
		data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
		data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");

		char   buffer[50];
		sprintf(buffer, "relestatus=%d,status=%s", shield.hearterActuator.getReleStatus(), statusStr[shield.hearterActuator.getStatus()]);
		data += String(buffer);

		data += F("</body></html>");
		client.println(data);
	}

	logger.print(tag, F("\n\t<<end show rele\n"));
}

/*void flash() {

	logger.println(tag, F("FLASH - start -----"));

	//time_t currentTime = millis();

	shield.checkTemperatures();
	
	// se il sensore attivo è quello locale aggiorna lo stato
	// del rele in base alla temperatur del sensore locale
	if (!shield.hearterActuator.sensorIsRemote())
		shield.hearterActuator.updateReleStatus();
	
	logger.print(tag, F("\n\tFLASH - END \n\t"));
}*/

void loop()
{
	ArduinoOTA.handle();  // questa chiamata deve essere messa in loop()
	
	wdt_enable(WDTO_8S);
	
	//////////////////
	String page, param;
	client = server.available();
	HttpHelper http;
	bool res = false;
	if (client) {
		res = http.getNextPage(&client, &server, &page, &param);

		if (res) {

			String data = "";
			if (page.equalsIgnoreCase("main")) {
				data = showMain(param);
				showPage(data);
			}
			else if (page.equalsIgnoreCase("setting")) {
				data = showSettings(param);
				showPage(data);
			}
			else if (page.equalsIgnoreCase("iodevices")) {
				data = showIODevices(param);
				showPage(data);
			}
			else if (page.equalsIgnoreCase("chstt")) {
				showChangeSettings(param);
			}
			else if (page.equalsIgnoreCase("chiodevices")) {
				showChangeIODevices(param);
			}
			else if (page.equalsIgnoreCase("wol")) {
				showwol(param);
			}
			else if (page.equalsIgnoreCase("rele")) {
				showRele(param);
			}
			else if (page.equalsIgnoreCase("power")) {
				showPower(param);
			}
			else if (page.equalsIgnoreCase("temp")) {
				setRemoteTemperature(param);
			}
			else if (page.equalsIgnoreCase("status")) {
				data = getJsonStatus();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("sensorstatus")) {
				data = getJsonSensorsStatus();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("actuatorsstatus")) {
				data = getJsonActuatorsStatus();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("reset")) {
				data = softwareReset();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("register")) {
				data = registerShield();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("index.html")) {
				data = showIndex();
				showPage(data);
			}
			else if (page.equalsIgnoreCase("prova3.html")) {
				getFile(page);
			}
			else if (page.equalsIgnoreCase("ESP8266.css")) {
				getFile(page);
			}
			else if (page.equalsIgnoreCase("download")) {
				download();
			}
		}
		// give the web browser time to receive the data
		//delay(20);
		client.stop();

		return;
	}
	
	
	/////////////////
	
	//Command command;
	
	if (shield.id >= 0 && !WiFi.localIP().toString().equals(shield.localIP)) {

		Command command;
		logger.println(tag, "IP ADDRESS ERROR - re-register shield");
		command.registerShield(shield);
		shield.localIP = WiFi.localIP().toString();
		return;
	}

	shield.checkActuatorsStatus();
	shield.checkSensorsStatus();
	
	
	unsigned long currMillis = millis();	
	unsigned long timeDiff = currMillis - lastFlash;
	if (timeDiff > flash_interval) {

		lastFlash = currMillis;
		if (shield.id <= 0) {
			Command command;
			logger.println(tag, F("ID NON VALIDO"));
			shield.id = command.registerShield(shield);
		}

		//shield.checkSensorsStatus();
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

void showPage(String data) {

	logger.println(tag, "showPage ");

	client.println(data);
	delay(1000);
	//client.stop();
}

String showIODevices(String param)
{
	logger.println(tag, F("showIODevices "));

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");

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

		/*data += F("<option value='1' >");
		data += Shield::getIoDevicesTypeName(1);
		data += F("</option>");

		data += F("<option value='2' >");
		data += Shield::getIoDevicesTypeName(2);
		data += F("</option>");*/


		data += F("</select>");

		data += F("</td></tr>");
	}

	data += F("</table><input type='submit' value='save'/></form>");


	data += F("</body></html>");

	return data;

}

String showSettings(String param)
{
	logger.println(tag, F("showSettings "));

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");

	data += F("<font color='#53669E' face='Verdana' size='2'><b>Impostazioni </b></font>\r\n<form action='/chstt' method='POST'><table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// local port
	data += F("<tr><td>Local port</td><td><input type='num' name='localport");
	data += F("' value='");
	data += shield.localPort;
	data += F("' size='4' maxlength='4'> </td></tr>");
	// board name
	data += F("<tr><td>Board name</td><td><input type='num' name='boardname' value='");
	data += String(shield.boardname);
	data += F("' size='");
	data += String(shield.boardnamelen - 1);
	data += F("' maxlength='");
	data += String(shield.boardnamelen - 1);
	data += F("'> </td></tr>");
	// ssid
	data += F("<tr><td>SSID</td><td><input type='num' name='ssid");
	data += F("' value='");
	data += String(shield.networkSSID);
	data += F("' size='32' maxlength='32'> </td></tr>");
	// password
	data += F("<tr><td>password</td><td><input type='num' name='password");
	data += F("' value='");
	data += String(shield.networkPassword);
	data += F("' size='96' maxlength='96'> </td></tr>");
	// server name
	data += F("<tr><td>Server name</td><td><input type='num' name='servername' value='");
	data += String(shield.servername);
	data += F("' size='");
	data += (shield.servernamelen - 1);
	data += F("' maxlength='");
	data += (shield.servernamelen - 1);
	data += F("'> </td></tr>");
	// server port
	data += F("<tr><td>Server port</td><td><input type='num' name='serverport");
	data += F("' value='");
	data += String(shield.serverPort);
	data += F("' size='4' maxlength='4'> </td></tr>");

	data += F("</table><input type='submit' value='save'/></form>");
		

	data += F("</body></html>");

	return data;

}

String showMain(String param)
{
	logger.println(tag, F("showMain "));

	char buffer[200];

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");
	// comandi
	data += F("\n<font color='#53669E' face='Verdana' size='2'><b>Webduino - ");
	data += logger.getStrDate();
	data += F("</b></font>");
	data += F("\r\n<table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// power
	data += F("<tr><td>Power ");
	if (shield.hearterActuator.getStatus() == Program::STATUS_DISABLED) {
		data += F("OFF</td><td><form action='/power' method='POST'>");
		data += F("<input type='hidden' name='status' value='1' >");
		data += F("<input type='submit' value='on'></form>");
	}
	else {
		data += F("ON</td><td><form action='/power' method='POST'>");
		data += F("<input type='hidden' name='status' value='0' >");
		data += F("<input type='submit' value='off'></form>");
	}
	//client.print(F("<input type='submit' value='set'></form>"));
	data += F("</td></tr>");
	// status & rele	
	data += "<tr><td>Actuator:</td><td>";
	data += statusStr[shield.hearterActuator.getStatus()];
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
		data += String(onStatusDuration/60);
		data += F(" duration:");
		data += String(duration);
		data += F(" ");
		data += String(duration / 60L);
		data += F(" remainingTime:");
		data +=String(remainingTime);
		data += F(" ");
		data += String(remainingTime / 60L);

		/*sprintf(buffer, " duration:(%2d) ", shield.hearterActuator.programDuration);
		data += String(buffer);
		sprintf(buffer, "%02d:", hr);
		data += String(buffer);
		sprintf(buffer, "%02d:", mn);
		data += String(buffer);
		sprintf(buffer, "%02d", sec);
		data += String(buffer);*/

		
		/*sprintf(buffer, " tempo rimanente:(%2d) ", remainingTime);
		data += String(buffer);
		sprintf(buffer, "%02d:", hr);
		data += String(buffer);
		sprintf(buffer, "%02d:", mn);
		data += String(buffer);
		sprintf(buffer, "%02d", sec);
		data += String(buffer);

		hr = onStatusDuration / 3600L;
		mn = (onStatusDuration - hr * 3600L) / 60L;
		sec = (onStatusDuration - hr * 3600L) % 60L;
		sprintf(buffer, " programma attivo da:(%2d) ", onStatusDuration);
		data += String(buffer);
		sprintf(buffer, "%02d:", hr);
		data += String(buffer);
		sprintf(buffer, "%02d:", mn);
		data += String(buffer);
		sprintf(buffer, "%02d", sec);
		data += String(buffer);*/

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

	} else if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {
		
		data += F("program manual  off");
	}
	data += F("</td></tr>");
	// Manual
	data += F("<tr><td>Manual-- ");
	if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO || shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F("<tr><td>Manual ON</td><td>");
		
		if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO) {
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
	/*else {
		data += F("</td><td>--");
	}*/
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
	// targetTemperature
	/*data += "<tr><td>Target Temperature: </td><td>" + String(targetTemperature)  + "°C</td></tr>";
	// remote Sensor
	data += "<tr><td>Remote Sensor: </td><td>" + String(temperatureSensor) + "(" +
		String(remoteTemperature)+ "°C)</td></tr>";
	// remote temperature
	//sprintf(buffer, "<tr><td>Remote Temperature: </td><td>%d.%02d</td></tr>", (int)remoteTemperature, (int)(remoteTemperature * 100.0) % 100);
	//data += String(buffer);
	// program update
	sprintf(buffer, "<tr><td>Last program or temperature update: </td><td>%d msec</td></tr>", (millis() - last_RemoteSensor));
	data += String(buffer);*/
	// wol
	//data += F("<tr><td>WOL</td><td><form action='/wol' method='POST'><input type='submit' value='send'></form></td></tr>");
	
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
	//data += "<tr><td>ID</td><td>" + String(shield.id) + "</td></tr>";
	data += "<tr><td>Shield Id</td><td>" + String(shield.id) + "<form action='/register' method='POST'><input type='submit' value='register'></form></td></tr>";

	data += F("</table>");

	data += String(Versione);

	data += "\nEPROM_Table_Schema_Version=" + String(EPROM_Table_Schema_Version);

	data += F("</body></html>");
	
	return data;
}

void showChangeIODevices(String param) {

	logger.println(tag, F("showChangeIODevices "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];
		
	int val;
	
	for (int i = 0; i < Shield::getMaxIoDevices(); i++) {
		
		char buffer[20];
		String str = "iodevice" + String(i + 1);
		str.toCharArray(buffer, sizeof(buffer));
		//logger.print(tag, "\n\tiodevice=");
		//logger.print(tag, buffer);
		val = parsePostdata(databuff, buffer, posdata);
		if (val != -1) {
			logger.print(tag, "\n\t" + String(buffer) + "=");
			logger.print(tag, String(val));
			Shield::setIODevice(i, val);
		}
	}
	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main'><title>Timer</title></head><body></body></html>");
	data += F("</body></html>");
	client.println(data);
	
	writeEPROM();
	client.stop();
}

void showChangeSettings(String param) {

	logger.println(tag, F("showChangeSettings "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	// localport
	val = parsePostdata(databuff, "localport", posdata);
	if (val != -1) {
		shield.localPort = val;
		logger.print(tag, "\n\tlocalPort=");
		logger.print(tag, shield.localPort);
	}
	// ssid
	if (val != -1) {
		val = parsePostdata(databuff, "ssid", posdata);
		memccpy_P(shield.networkSSID, posdata, '\0', sizeof(shield.networkSSID));
		logger.print(tag, "\n\tnetworkSSID=");
		logger.print(tag, shield.networkSSID);
	}
	// password
	if (val != -1) {
		val = parsePostdata(databuff, "password", posdata);
		memccpy_P(shield.networkPassword, posdata, '\0', sizeof(shield.networkPassword));
		logger.print(tag, "\n\tnetworkPassword=");
	}
	// server name
	val = parsePostdata(databuff, "servername", posdata);
	if (val != -1) {
		memccpy_P(shield.servername, posdata, '\0', shield.servernamelen);
		logger.print(tag, "\n\tservername ");
		logger.print(tag, shield.servername);
	}
	// server port
	val = parsePostdata(databuff, "serverport", posdata);
	if (val != -1) {
		logger.print(tag, "\n\tserver port ");
		logger.print(tag, val);
		shield.serverPort = val;
	}
	// board name
	val = parsePostdata(databuff, "boardname", posdata);
	if (val != -1) {
		memccpy_P(shield.boardname, posdata, '\0', shield.boardnamelen);
		logger.print(tag, "\n\tboardname=");
		logger.print(tag, shield.boardname);
	}
	// sensor names

	for (int i = 0; i < shield.sensorList.count; i++) {
		DS18S20Sensor*  sensor = (DS18S20Sensor*)shield.sensorList.get(i);
		char buffer[20];
		String str = "sensor" + String(i + 1);
		str.toCharArray(buffer, sizeof(buffer));
		logger.print(tag, "\n\tbuffer=");
		logger.print(tag, buffer);
		val = parsePostdata(databuff, buffer, posdata);
		if (val != -1) {
			sensor->sensorname = String(posdata);
			logger.print(tag, "\n\t" + String(buffer) + "=");
			logger.print(tag, String(posdata));
			logger.print(tag, "\n\tsensor->sensorname=");
			logger.print(tag, sensor->sensorname);

		}
	}
	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main'><title>Timer</title></head><body></body></html>");
	data += F("</body></html>");
	client.println(data);
	//client.stop();

	// reimposta servername e port (sono due var static in Command quindi bast chiamare costruttore)
	Command command;
	command.setServer(shield.servername, shield.serverPort);

	writeEPROM();
	client.stop();
}

int len(const char* data) {

	int l = 0;
	while (data[l] != '\0')
		l++;
	return l;
}

int parsePostdata(const char* data, const char* param, char* value) {


	int pos = findIndex(data, param);

	if (pos != -1) {

		int i = 0;
		char c;
		while (i < len(data) && i < maxvaluesize) {
			if (data[i + pos + len(param) + 1] == '&' || data[i + pos + len(param) + 1] == '\0')
				break;
			value[i] = data[i + pos + len(param) + 1];
			i++;
		}
		//if (i < maxvaluesize) value[i] = '\0';
		/*if (i < maxvaluesize)*/ value[i] = '\0';
		int val = 0;

		for (int k = 0; k < i; k++) {

			val = (val * 10) + value[k] - 48;
		}
		return val;
	}
	return -1;
}

void getPostdata(char *data, int maxposdata) {

	//Serial.print(F("getPostdata "));

	int datalen = 0;

	if (client.findUntil("Content-Length:", "\n\r"))
	{
		datalen = client.parseInt();
	}

	delay(400);
	if (client.findUntil("\n\r", "\n\r"))
	{
		;
	}
	delay(400);
	client.read();

	if (datalen >= maxposdata) {
		logger.println(tag, F("maxposdat to long"));
	}

	int i = 0;
	while (i < datalen && i < maxposdata) {
		data[i] = client.read();
		//Serial.print(data[i]); // ailitare questa riga per vedere il contenuto della post
		delay(2);
		i++;
	}

	//Serial.println("");
	//Serial.print("datalen ");
	//Serial.print(datalen);
	if (i < maxposdata)
		data[i] = '\0';
}

int findIndex(const char* data, const char* target) {

	boolean found = false;
	int i = 0;
	while (data[i] != '\0') {
		i++;
	}
	i = 0;
	int k = 0;
	while (data[i] != '\0') {

		if (data[i] == target[0]) {
			found = true;
			k = 0;
			while (target[k] != '\0') {

				if (data[i + k] == '\0')
					return -1;
				if (data[i + k] != target[k]) {
					found = false;
					break;
				}
				k++;
			}
			if (found == true)
				return i;
		}
		i++;
	}
	return -1;
}

String getJsonStatus()
{
	logger.print(tag, F("\n\tCALLED getJsonStatus"));

	String data;
	data += "";
	data += "HTTP/1.0 200 OK\r\nContent-Type: application/json; ";
	data += "\r\nPragma: no-cache\r\n\r\n";

	data += shield.hearterActuator.getJSON();
	/*data += F("{\"id\":");
	data += String(shield.id);

	data += F(",\"remotetemperature\":");
	data += String(shield.hearterActuator.getRemoteTemperature());

	char buf[50];
	sprintf(buf, ",\"status\":\"%s\"", statusStr[shield.hearterActuator.getStatus()]);
	data += String(buf);

	data += String(F(",\"relestatus\":"));
	if (shield.hearterActuator.getReleStatus())
		data += F("true");
	else
		data += F("false");

	data += F(",\"name\":");
	data += shield.hearterActuator.sensorname;

	if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE 
		|| shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_AUTO 
		|| shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {

		data += F(",\"duration\":");
		data += String(shield.hearterActuator.programDuration);

		int remainingTime = shield.hearterActuator.programDuration - (millis() - shield.hearterActuator.programStartTime);
		data += String(F(",\"remaining\":"));
		data += String(remainingTime);

		data += F(",\"localsensor\":");
		if (!shield.hearterActuator.sensorIsRemote())
			data += F("true");
		else
			data += F("false");

		if (shield.hearterActuator.getStatus() == Program::STATUS_MANUAL_OFF) {
			data += F(",\"target\":");
			data += String(shield.hearterActuator.getTargetTemperature());
		}

		if (shield.hearterActuator.getStatus() == Program::STATUS_PROGRAMACTIVE) {
			data += F(",\"program\":");
			data += String(shield.hearterActuator.getActiveProgram());

			data += F(",\"timerange\":");
			data += String(shield.hearterActuator.getActiveTimeRange());
		}

	}
	data += F("}");*/

	//logger.print(tag, F("\n\t json="));
	//logger.print(tag, data);
	return data;
}

String getJsonSensorsStatus()
{
	logger.println(tag, F("CALLED getJsonSensorStatus"));

	String data;
	data += "";
	data += "HTTP/1.0 200 OK\r\nContent-Type: application/json; ";
	data += "\r\nPragma: no-cache\r\n\r\n";

	data += shield.getSensorsStatusJson();

	logger.println(tag, data);
	return data;
}

String getJsonActuatorsStatus()
{
	logger.println(tag, F("CALLED getJsonActuatorsStatus"));

	String data;
	data += "";
	data += "HTTP/1.0 200 OK\r\nContent-Type: application/json; ";
	data += "\r\nPragma: no-cache\r\n\r\n";

	data += shield.getActuatorsStatusJson();

	logger.println(tag, data);
	return data;
}

void setRemoteTemperature(String param) {

	logger.println(tag, F("called setRemoteTemperature: "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	
	parsePostdata(databuff, "temperature", posdata);
	String str = "";
	str += posdata;

	shield.hearterActuator.setRemoteTemperature(str.toFloat());
		
	String data;
	data += getJsonStatus();

	client.println(data);
}

void showPower(String GETparam) {

	logger.println(tag, F("showPower "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	// status
	val = parsePostdata(databuff, "status", posdata);
	logger.print(tag, F("\n\tstatus="));
	logger.print(tag, val);

	logger.print(tag, F("\n\tprogramSettings.currentStatus="));
	logger.print(tag, shield.hearterActuator.getStatus());

	if (val == 1 && shield.hearterActuator.getStatus() == Program::STATUS_DISABLED) {
		shield.hearterActuator.setStatus(Program::STATUS_IDLE);
		shield.hearterActuator.enableRele(true);

	}
	else if (val == 0) {
		shield.hearterActuator.setStatus(Program::STATUS_DISABLED);
		shield.hearterActuator.enableRele(false);
	}

	/*String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='2; url=/main?msg=2'><title>postStatus</title></head><body>");
	char   buffer[50];
	sprintf(buffer, "relestatus=%d,status=%s", programSettings.releStatus, statusStr[programSettings.currentStatus]);
	data += String(buffer);
	data += F("</body></html>");*/

	String data = "HTTP/1.1 200 OK\r\nContent-Type: text/xml\n\n<result><rele>" + String(shield.hearterActuator.getReleStatus()) + "</rele>" +
		"<status>" + String(shield.hearterActuator.getStatus()) + "</status></result>";

	client.println(data);

}


/*void showPower2(String GETparam) {

	logger.println(tag, F("showPower "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	// status
	val = parsePostdata(databuff, "status", posdata);
	logger.print(tag, F("\n\tstatus="));
	logger.println(tag, val);


	if (val == 1 && programSettings.currentStatus == Program::STATUS_DISABLED) {
		programSettings.currentStatus = Program::STATUS_IDLE;
		enableRele(false);

	}
	else if (val == 0) {
		programSettings.currentStatus = Program::STATUS_DISABLED;
		enableRele(false);
	}

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='2; url=/main?msg=2'><title>postStatus</title></head><body>");

	char   buffer[50];
	sprintf(buffer, "relestatus=%d,status=%s", programSettings.releStatus, statusStr[programSettings.currentStatus]);
	data += String(buffer);

	data += F("</body></html>");
	client.println(data);

}*/

//String cssfile = "";
String getFile(String filename) {

	logger.println(tag, F("showprova"));
	char* s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

	String header = "";

	if (filename.endsWith(".css"))
		header += F("HTTP/1.1 200 OK\r\nContent-Type: text/css\n\n");
	else
		header += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n");

	File f = SPIFFS.open("/" + filename, "r");
	if (!f) {
		Serial.println("File /" + filename + " doesn't exist");
		return "";
	}
	else if (filename.equals("ESP8266.css") /*&& !cssfile.equals("")*/) {
		client.println(header);

		int len = strlen_P(cssfile);
		int i = 0, k = 0;
		//char buffer[100];
		while (i < len) {

			//i += strlcpy_P(buffer, (char*)pgm_read_word(cssfile+i));
			String str = "";

			for (k = 0; k < 1000; k++)
			{

				if (i + k >= len) {
					break;
				}

				//strlcpy_P(buffer, (char*)pgm_read_word(&(string_table[i])));
				//*(buffer+k) = pgm_read_byte_near(cssfile + i + k);
				char s = pgm_read_byte_near(cssfile + i + k);
				str += s;

			}
			i += k;
			client.print(str);
		}


		//client.println(cssfile);
		//delay(2000);
	}
	else {
		client.println(header);
		// we could open the file
		String line = "";
		while (f.available()) {
			//Lets read line by line from the file
			line += f.readStringUntil('\n');
			//if (line.length() > 10000) {
			//client.println(line);
			//Serial.println(line);
			//	line = "";
			//}
		}
		client.println(line);
		//delay(10);
		//if (filename.equals("ESP8266.css"))
		//	cssfile = line;
	}
	f.close();



#ifdef dopo
	char* s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html>\
		<head><link href=\"http://192.168.1.3:8080/webduino/ESP8266.css\" rel=\"stylesheet\"/>\
		<title></title></head><body><!--Header--->\
		<h1 id = \"head\">Steal My Admin Template</h1>\
		<ul id = \"navigation\">\
		<li><span class = \"active\">Overview</span></li>\
		<li><a href =\"#\" class=\"button hvr-underline-from-center\">News</a></li>\
		<li><a href =\"#\" class=\"button hvr-underline-from-center\">Checkout</a></li>\
		<li><a href =\"#\" class=\"button hvr-underline-from-center\">Checkout</a></li>\
		<li><a href =\"#\" class=\"button hvr-underline-from-center\">Users</a></li>\
		</ul>\
		<!--Content--->\
		<div id = \"content\" class=\"container_16 clearfix\">\
		<div class=\"grid_5\">\
		<div class=\"box\">\
		<h2>Mathew</h2>\
		<p><strong>Last Signed In : </strong> Wed 11 Nov, 7 : 31<br/><strong>IP Address : </strong> 192.168.1.101</p>\
		</div>\
		<div class=\"box\">\
		<h2>Files</h2>\
		<table>\
		<tbody>\
		<tr>\
		<td>Newton 2</td>\
		<td>8 / 10</td>\
		</tr>\
		<tr>\
		<td>Wicked Twister</td>\
		<td>9 / 10</td>\
		</tr>\
		<tr>\
		<td>Forester</td>\
		<td>9.12 / 10</td>\
		</tr>\
		<tr>\
		<td>Sabertooth</td>\
		<td><div>8.9 / 10 <input type = \"submit\" value = \"post\"></div></td>\
		</tr>\
		</tbody>\
		</table>\
		</div>\
		</div>\
		</div>\
		<!--Footer--->\
		<div id = \"foot\">\
		<div class = \"container_16 clearfix\">\
		<div class = \"grid_16\">\
		<a href = \"#\">Contact Me</a>\
		</div>\
		</div>\
		</div>\
		</body>\
		</html>";
#endif
	return String(s);
}
