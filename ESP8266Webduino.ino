
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "user_interface.h"
#include "wol.h"
#include "HttpHelper.h"
#include "JSON.h"
#include "OneWireSensors.h"
#include "Settings.h"
#include "Command.h"
#include "Program.h"

extern uint8_t provapin = D4;

#define Versione "0.99"
const char SWVERSION[] = "1.01";

#define			notification_statuschange		1
#define			notification_restarted			2
#define			notification_programend			3
#define			notification_relestatuschange	4
#define			notification_offline			5

#define			relestatus_off		0
#define			relestatus_on		1
#define			relestatus_disabled	2
#define			relestatus_enabled	3


extern const char* statusStr[] = { "unused", "idle", "program", "manual", "disabled", "restarted" };

OneWireSensors ows = OneWireSensors();
Settings settings;
Program programSettings;

void initEPROM();
void readEPROM();
void writeEPROM();

int memoryFree();
void getPostdata(char *data, int maxposdata);
int parsePostdata(const char* data, const char* param, char* value);
void getStatus(char* GETparam);
//float readTemperature();
//float getAverageTemperature(int n);
int findIndex(const char* data, const char* target);
void showMain(boolean isPost, char* param);
void showChangeSettings(char* GETparam);
void showPower(char* GETparam);
void setRemoteTemperature(char* GETparam);
bool sendNotification(int type, int value);
//bool post(char* host, int port, char* path, char* param, int len);
void enableRele(boolean on);
//boolean registerShield();
//boolean sendSensorsStatus();
//float readTemperatures();

#ifdef dopo
IPAddress pingAddr(192, 168, 1, 1); // ip address to ping
#endif



const char* ssid = "xxBUBBLES";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;

const int relePin = D5; // rel? pin
const bool RELE_ON = LOW;//LOW
const bool RELE_OFF = HIGH;//LOW

int activeProgram = -1;
int activeTimerange = -1;

const int maxposdataChangeSetting = 150;
char databuff[maxposdataChangeSetting];

#define P(name) static const prog_uchar name[] PROGMEM // declare a static string
const byte EEPROM_ID = 0x99; // used to identify if valid data in EEPROM
const int ID_ADDR = 0; // the EEPROM address used to store the ID
const int TIMERTIME_ADDR = 1; // the EEPROM address used to store the pin

/*   da eliminare*/
byte mac[] = { 0x00, 0xA0, 0xB0, 0xC0, 0xD0, 0x90 };
byte ip[] = { 192, 168, 1, 95 };

byte dhcp = 1;
/*  fine*/


const int MAX_PAGE_NAME_LEN = 12;
const int MAX_PARAM_LEN = 12;//12;

const int maxposdata = 101; // massimo numero di caratteri della variabile posdata
const int maxvaluesize = maxposdata - 1/*10*/; // massimo numero di digit del valore di una variabile nel POS data


bool statusChangeSent = true;

float targetTemperature = 0.0;
float remoteTemperature = 0;

unsigned long lastFlash = 0;
const int flash_interval = 30000;
unsigned long lastStatusChange = 0;
const int lastStatusChange_interval = 600000; // timeout retry invio status change
unsigned long lastNotification = 0;
const int Notification_interval = 20000;

#ifdef dopo
SOCKET pingSocket = 0;
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));
#endif
int offlineCounter = 0;

unsigned long last_RemoteSensor = 0;
unsigned long remoteSensorTimeout = 36000; // tempo dopo il quale il programa ssi disattiva

// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

const int temperaturePin = 3;

bool netwokStarted = false;

#define     sensor_local    0
#define     sensor_remote   1
int temperatureSensor = sensor_local;

int sendRestartNotification = 0;

bool shieldRegistered = false; // la shield si registra all'inizio e tutte le volte che va offline

unsigned long totalConsumptionTime = 0;
unsigned long lastConsumptionEnableTime = 0;
unsigned long ConsumptionStartTime;

void writeEPROM() {

	Serial.println("write EPROM");
		
	int addr = TIMERTIME_ADDR;
	EEPROM.write(ID_ADDR, EEPROM_ID); // write the ID to indicate valid data
	byte hiByte;
	byte loByte;

	// id
	EEPROM.write(addr++, settings.id);
	// local port
	hiByte = highByte(settings.localPort);
	loByte = lowByte(settings.localPort);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	// ssid
	Serial.print("networkSSID=");
	Serial.println(settings.networkSSID);
	int res = EEPROM_writeAnything(addr, settings.networkSSID);
	addr += res;
	// password
	res = EEPROM_writeAnything(addr, settings.networkPassword);
	addr += res;
	// server name
	res = EEPROM_writeAnything(addr, settings.servername);
	addr += res;
	// server port
	hiByte = highByte(settings.serverPort);
	loByte = lowByte(settings.serverPort);
	EEPROM.write(addr++, hiByte);
	EEPROM.write(addr++, loByte);
	// board name
	res = EEPROM_writeAnything(addr, settings.boardname);
	addr += res;
	// sensor names
	for (int i = 0; i < ows.sensorCount; i++) {
		res = EEPROM_writeAnything(addr, ows.sensorname[i]);
		addr += res;
	}
	EEPROM.commit();
}

void readEPROM() {

	Serial.println("read EPROM");

	byte hiByte;
	byte lowByte;
	int addr = TIMERTIME_ADDR;
	// id
	settings.id = EEPROM.read(addr++);
	// local port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	settings.localPort = word(hiByte, lowByte);
	// ssid
	int res = EEPROM_readAnything(addr, settings.networkSSID);
	Serial.println("networkSSID=");
	Serial.println(settings.networkSSID);
	addr += res;
	// password
	res = EEPROM_readAnything(addr, settings.networkPassword);
	Serial.println("networkPassword=");
	Serial.println(settings.networkPassword);
	addr += res;
	//server name
	res = EEPROM_readAnything(addr, settings.servername);
	addr += res;
	// server port
	hiByte = EEPROM.read(addr++);
	lowByte = EEPROM.read(addr++);
	settings.serverPort = word(hiByte, lowByte);
	// board name
	res = EEPROM_readAnything(addr, settings.boardname);
	addr += res;
	// sensor names
	for (int i = 0; i < ows.sensorCount; i++) {
		res = EEPROM_writeAnything(addr, ows.sensorname[i]);
		addr += res;
	}
}

void initEPROM()
{
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

int memoryFree()
{
#ifdef dopo
	int freeValue;

	if ((int)__brkval == 0)
		freeValue = ((int)&freeValue) - ((int)&__bss_end);
	else
		freeValue = ((int)&freeValue) - ((int)__brkval);
	return freeValue;
#elseif
	return 0;
#endif
}

int testWifi(void) {
	int c = 0;
	Serial.println("Waiting for Wifi to connect");
	while (c < 20) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("WiFi connected");
			return(20);
		}
		delay(500);
		Serial.print(WiFi.status());
		c++;
	}
	Serial.println("Connect timed out, opening AP");
	return(10);
}

const char *ssidAP = "ES8266";
const char *passwordAP = "thereisnospoon";

void setupAP(void) {

	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0)
		Serial.println("no networks found");
	else
	{
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
			delay(10);
		}
	}
	Serial.println("");
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
	for (int i = 0; i < sizeof(settings.MAC_array); ++i) {
		if (i > 0) 
			ssidName += ":";
		ssidName += settings.MAC_array[i];
	}
	char buffer[100];
	ssidName.toCharArray(buffer, 100);
	Serial.print("ssidAP buffer= ");
	Serial.println(buffer);
	WiFi.softAP(buffer, passwordAP);
	//WiFi.softAP(ssidAP, passwordAP);

	Serial.println("softap");
	Serial.println("");
	Serial.println("over");
}

void setup()
{
	Serial.begin(115200);
	delay(10);

	Serial.println("");
	Serial.println("");
	Serial.println("--*******************************************--");
	Serial.print("starting.... ");

	String str = "Versione ";
	str += Versione;
	Serial.print(str);

	// get MAC Address
	Serial.println("");
	Serial.print("MAC Address ");
	WiFi.macAddress(settings.MAC_array);
	for (int i = 0; i < sizeof(settings.MAC_array); ++i) {
		if (i > 0) sprintf(settings.MAC_char, "%s:", settings.MAC_char);
		sprintf(settings.MAC_char, "%s%02x", settings.MAC_char, settings.MAC_array[i]);

	}
	Serial.println(settings.MAC_char);

	// rele
	pinMode(relePin, OUTPUT);
	enableRele(false);
	ConsumptionStartTime = millis();

	initEPROM();

	//_sensors.begin();
	//discoverOneWireDevices();

	//ows = new OneWireSensors();
	ows.discoverOneWireDevices();
	// Connect to WiFi network
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(settings.networkSSID);
	Serial.println(settings.networkPassword);

	WiFi.begin(settings.networkSSID, settings.networkPassword);
	//WiFi.begin(ssid, password);
	if (testWifi() == 20) {
		// Start the server
		server.begin();
		Serial.println("Server started");
		// Print the IP address
		Serial.println(WiFi.localIP());

		wdt_disable();
		wdt_enable(WDTO_8S);

		netwokStarted = true;
		programSettings.currentStatus = Program::STATUS_IDLE;

		Command command;
		shieldRegistered = command.registerShield(settings,ows);
		return;
	
	// Start the server
	} else {
		Serial.println("\nIMPOSSIBILE COLLEGARSI ALLA RETE\n");
		setupAP();

		server.begin();
		Serial.println("Local server started...192.168.4.1");
		// Print the IP address
		wdt_disable();
		wdt_enable(WDTO_8S);

		netwokStarted = false;
		programSettings.currentStatus = Program::STATUS_DISABLED;
	}
}



void enableRele(boolean on) {

	if (programSettings.releStatus) {
		totalConsumptionTime += (millis() - lastConsumptionEnableTime);
	}

	programSettings.oldReleStatus = programSettings.releStatus;
	if (on) {
		Serial.println(F("rele on"));
		digitalWrite(relePin, RELE_ON);
		programSettings.releStatus = true;

		lastConsumptionEnableTime = millis();
	}
	else {
		Serial.println(F("rele off"));
		digitalWrite(relePin, RELE_OFF);
		programSettings.releStatus = false;
	}

}

void showwol(char* GETparam) {

	//Serial.println(F("showwol "));
	//int ntimer = parsePostdata(GETparam, "prg");
	//int ret = parseChangeTimer(ntimer);

	wol* w = new wol();
	w->init();
	w->wakeup();
	delete w;

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
	data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>WOL</title></head><body>");
	data += F("</body></html>");
	client.println(data);
	client.stop();
}

void showRele(char* GETparam) {

	Serial.println(F("showRele "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];
	int val;
	// status
	val = parsePostdata(databuff, "status", posdata);
	Serial.print(F("status="));
	Serial.println(val, DEC);
	// duration
	long duration = parsePostdata(databuff, "duration", posdata);
	Serial.print(F("duration="));
	Serial.println(duration, DEC);
	duration = duration * 60 * 1000;
	Serial.print(F("duration * 60 * 1000 ="));
	Serial.println(duration, DEC);
	// temperature
	int res = parsePostdata(databuff, "target", posdata);
	//if (res != -1) {
	String str = "";
	str += posdata;
	targetTemperature = str.toFloat();
	Serial.print(F("targetTemperature="));
	Serial.println(targetTemperature, DEC);
	// sensor
	temperatureSensor = parsePostdata(databuff, "sensor", posdata);
	Serial.print(F("sensor="));
	Serial.println(temperatureSensor, DEC);
	if (temperatureSensor == sensor_remote) {
		last_RemoteSensor = millis();
		Serial.print(F("last_RemoteSensor="));
		Serial.println(last_RemoteSensor, DEC);
	}
	// remote temperature
	res = parsePostdata(databuff, "temperature", posdata);
	if (res != -1) {
		str = "";
		str += posdata;
		remoteTemperature = str.toFloat();
		Serial.print(F("remoteTemperature="));
		Serial.println(remoteTemperature, DEC);
	}
	else {
		Serial.print(F("remoteTemperature MISSING"));
	}
	// program
	int program = parsePostdata(databuff, "program", posdata);
	Serial.print(F("program="));
	Serial.println(program, DEC);
	int timerange = parsePostdata(databuff, "timerange", posdata);
	Serial.print(F("timerange="));
	Serial.println(timerange, DEC);
	// manual
	int manual = parsePostdata(databuff, "manual", posdata);
	Serial.print(F("manual="));
	Serial.println(manual, DEC);
	// jsonRequest
	int json = parsePostdata(databuff, "json", posdata);
	Serial.print(F("json="));
	Serial.println(json, DEC);

	Serial.print(F("currentStatus="));
	Serial.println(programSettings.currentStatus, DEC);
	if (val == relestatus_on && programSettings.currentStatus != Program::STATUS_DISABLED) {

		if (manual != 1) {
			Serial.print(F("rele on"));
			if (programSettings.currentStatus != Program::STATUS_MANUAL) {
				Serial.print(F("not manual"));
				enableRele(true);
				programSettings.currentStatus = Program::STATUS_PROGRAMACTIVE;
				if (duration != -1)
					programSettings.programDuration = duration;
				else
					programSettings.programDuration = 30000;
				programSettings.programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
			}
		}
		else if (manual == 1) {
			Serial.print(F("manual"));
			enableRele(true);
			programSettings.currentStatus = Program::STATUS_MANUAL;
			if (duration != -1)
				programSettings.programDuration = duration;
			else
				programSettings.programDuration = 30000;
			programSettings.programStartTime = millis();

		}
	}
	else if (val == relestatus_off) {
		Serial.print(F("rele off"));
		if (programSettings.currentStatus == Program::STATUS_MANUAL) { // il progr aut � finito ma il disp � in manual mode
			if (manual == 1) {
				Serial.print(F("manual stop"));
				enableRele(false);
				programSettings.currentStatus = Program::STATUS_IDLE;
			}
			else {
				Serial.print(F("manual mode, impossibile fermare"));
			}
		}
		else if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE) {
			Serial.print(F("program active rele off"));
			enableRele(false);
			activeProgram = program;
			activeTimerange = timerange;
		}
		else if (programSettings.currentStatus == Program::STATUS_IDLE) {

			if (manual != 1) {
				Serial.print(F("rele off"));
				Serial.print(F("not manual"));
				enableRele(false);
				programSettings.currentStatus = Program::STATUS_PROGRAMACTIVE;
				if (duration != -1)
					programSettings.programDuration = duration;
				else
					programSettings.programDuration = 30000;
				programSettings.programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
			}
		}
	}
	else if (val == relestatus_disabled) {
		enableRele(false);
		programSettings.currentStatus = Program::STATUS_DISABLED;
	}
	else if (val == relestatus_enabled) {
		enableRele(false);
		programSettings.currentStatus = Program::STATUS_IDLE;
	}

	if (json == 1) {
		getStatus(NULL);
	}
	else {
		String data;
		data += "";
		data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html");
		data += F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main?msg=2'><title>rele</title></head><body>");

		char   buffer[50];
		sprintf(buffer, "relestatus=%d,status=%s", programSettings.releStatus, statusStr[programSettings.currentStatus]);
		data += String(buffer);

		data += F("</body></html>");
		client.println(data);
	}
}


void flash() {

	Serial.println(F(""));
	Serial.println(F("FLASH............ "));

	time_t currentTime = millis();



	settings.localTemperature = ows.readTemperatures();
	settings.localAvTemperature = ows.getAverageTemperature(0);

	Serial.print(" ConsumptionStartTime=");
	Serial.println(ConsumptionStartTime);
	Serial.print(" totalConsumptionTime=");
	Serial.println(totalConsumptionTime);
	Serial.print(" lastConsumptionEnableTime=");
	Serial.println(lastConsumptionEnableTime);

	Serial.print(" targetTemperature=");
	Serial.println(targetTemperature);
	Serial.print("localTemperature=");
	Serial.println(settings.localTemperature, DEC);
	Serial.print("sensor_local=");
	Serial.println(sensor_local);
	Serial.print(" localAvTemperature=");
	Serial.println(settings.localAvTemperature);
	Serial.print(" oldLocalAvTemperature=");
	Serial.println(settings.oldLocalAvTemperature);
	Serial.print("active temperatureSensor=");
	Serial.println(temperatureSensor, DEC);
	Serial.print(" remoteTemperature=");
	Serial.println(remoteTemperature);

	if (programSettings.currentStatus == Program::STATUS_MANUAL) {

		Serial.print("STATUS_MANUAL");
		// se stato manuale accendi il rel� se sensore == locale e temperatura sensore locale < temperatura target oppure
		// se sensore == remoto e temperature sensore remoto < temperatura target 
		if ((temperatureSensor == sensor_local && settings.localTemperature < targetTemperature) ||
			(temperatureSensor == sensor_remote && remoteTemperature < targetTemperature)) {

			Serial.println(F("-LOW TEMPERATURE"));
			enableRele(true);
		}
		else {
			Serial.println(F("-HIGH TEMPERATURE"));
			enableRele(false);
		}
	}
	else if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE) {

		Serial.print("STATUS_PROGRAMACTIVE");

		if (temperatureSensor != sensor_local) {
			Serial.print(F("-REMOTE SENSOR")); // non modificare stato relke, controllato remotamenre

		}
		else if (temperatureSensor == sensor_local) {
			Serial.print(F("-LOCAL SENSOR"));
			if (settings.localTemperature < targetTemperature) {

				Serial.println(F("-LOW TEMPERATURE"));
				enableRele(true);
			}
			else {
				Serial.println(F("-HIGH TEMPERATURE"));
				enableRele(false);
			}
		}
	}
	else{
		Serial.println("INACTIVE-rele OFF");
		enableRele(false);
	}
}







bool pingServer() {

#ifdef dopo
	Serial.println(F("ping server ...."));
	for (int i = 0; i < 4; i++) {
		Serial.print(pingAddr);
		Serial.print(".");
	}
	ICMPEchoReply echoReply = ping(pingAddr, 4);
	if (echoReply.status == SUCCESS)
	{
		offlineCounter = 0;
		Serial.println("ping success");
		return true;
	}
	else
	{
		Serial.println("ping unsuccess");
		offlineCounter++;

		if (offlineCounter > 15) {
			//reset
			for (int i = 0; i >= 0; i++)
			{
				;
			}
		}
		return false;
	}
#elseif
	return true;
#endif
}

void loop()
{
	wdt_enable(WDTO_8S);
	Command command;

	if (programSettings.oldReleStatus != programSettings.releStatus) {
		char buf[10];
		sprintf(buf, "relestatus=%d", programSettings.releStatus);
		//sendNotification(notification_relestatuschange, releStatus);
		command.sendActuatorStatus(settings, ows, programSettings);
		programSettings.oldReleStatus = programSettings.releStatus;
		return;
	}
	//
	if (programSettings.currentStatus != programSettings.oldcurrentstatus) {
		if (programSettings.currentStatus == Program::STATUS_DISABLED) {
			char buf[10];
			sprintf(buf, "status=%d", 0);
			//sendNotification(notification_statuschange, currentStatus);
			command.sendActuatorStatus(settings, ows, programSettings);
		}
		programSettings.oldcurrentstatus = programSettings.currentStatus;
		return;
	}

	unsigned long currMillis = millis();


	if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE || programSettings.currentStatus == Program::STATUS_MANUAL) {

		if (temperatureSensor != sensor_local && (currMillis - last_RemoteSensor) > remoteSensorTimeout) {
			Serial.println("REMOTE SENSOR TIMEOUT");
			//sendNotification(notification_programend, (currMillis - last_RemoteSensor));
			enableRele(false);
			// � iniutile mandare un sendstatus perch� tanto cambia lo stato dopo e verrebbe inviato due volte
			remoteTemperature = 0;
			programSettings.currentStatus = Program::STATUS_IDLE;
			return;

		}
		else if (currMillis - programSettings.programStartTime > programSettings.programDuration) {
			Serial.print("END PROGRAM");
			//sendNotification(notification_programend, 1);
			enableRele(false);
			// � iniutile mandare un sendstatus perch� tanto cambia lo stato dopo e verrebbe inviato due volte
			programSettings.currentStatus = Program::STATUS_IDLE;
			return;

		}
	}

	if (currMillis - lastFlash > flash_interval) {
		lastFlash = currMillis;
		flash();
		Command command;
		command.sendSensorsStatus(settings, ows);
		return;
	}

	if ((settings.oldLocalAvTemperature != settings.localAvTemperature && statusChangeSent == true) || (statusChangeSent == false && (currMillis - lastStatusChange) > lastStatusChange_interval)) {


		Serial.println("");
		Serial.println("SEND STATUS");
		Serial.print(" oldLocalAvTemperature=");
		Serial.println(settings.oldLocalAvTemperature, DEC);
		Serial.print(" localAvTemperature=");
		Serial.println(settings.localAvTemperature, DEC);
		Serial.print(" statusChangeSent=");
		Serial.println(statusChangeSent);

		Serial.print(" currMillis=");
		Serial.println(currMillis, DEC);
		Serial.print(" lastStatusChange=");
		Serial.println(lastStatusChange, DEC);

		Command command;
		if (command.sendActuatorStatus(settings, ows, programSettings)) {
			statusChangeSent = true;
			settings.oldLocalAvTemperature = settings.localAvTemperature;
		}
		else {
			statusChangeSent = false;
			lastStatusChange = currMillis;
		}
		return;
	}


	if (currMillis - lastNotification > Notification_interval) {

		lastNotification = currMillis;
		pingServer();
		return;
	}

	char buffer[MAX_PAGE_NAME_LEN + 1]; // additional character for terminating null
	char parambuffer[MAX_PARAM_LEN];

	client = server.available();
	if (client) {

		int type = 0;
		while (client.connected()) {

			if (client.available()) {
				// GET, POST, or HEAD
				memset(buffer, 0, sizeof(buffer)); // clear the buffer
				memset(parambuffer, 0, sizeof(parambuffer));
				if (client.readBytesUntil('/', buffer, MAX_PAGE_NAME_LEN)){
					Serial.println(buffer);
					if (strcmp(buffer, "GET ") == 0)
						type = 1;
					else if (strcmp(buffer, "POST ") == 0)
						type = 2;
					// look for the page name
					memset(buffer, 0, sizeof(buffer)); // clear the buffer
					int l;

					if (l = client.readBytesUntil(' ', buffer, MAX_PAGE_NAME_LEN))
					{
						Serial.println(l, DEC);
						Serial.println(buffer);
						l = findIndex(buffer, "?");
						int i = 0;
						if (l != -1) {
							while ((l + i) < MAX_PAGE_NAME_LEN && i < MAX_PARAM_LEN) {
								parambuffer[i] = buffer[l + i];
								i++;
							}
							buffer[l] = '\0';
						}
						else {
							;
						}
						//Serial.println(l, DEC);
						//Serial.println(buffer);
						//Serial.println(parambuffer);
						//Serial.println("-");

						if (strcmp(buffer, "main") == 0)
							showMain(type == 2, parambuffer);
						else if (strcmp(buffer, "chstt") == 0)
							showChangeSettings(parambuffer);
						else if (strcmp(buffer, "wol") == 0)
							showwol(parambuffer);
						else if (strcmp(buffer, "rele") == 0)
							showRele(parambuffer);
						else if (strcmp(buffer, "power") == 0)
							showPower(parambuffer);
						else if (strcmp(buffer, "temp") == 0)
							setRemoteTemperature(parambuffer);
						else if (strcmp(buffer, "status") == 0)
							getStatus(parambuffer);
						/*else
						unknownPage(buffer);*/
					}
				}
				break;
			}
		}
		// give the web browser time to receive the data
		delay(20);
		client.stop();
	}

	if (sendRestartNotification < 10) {
		pingServer(); // per qualche motivo se non faccio prima ping la notification non funziona e non si pu� mettere tutto in setup()
		Serial.print(F("SEND RESTART NOTIFICATION "));
		if (sendNotification(notification_restarted, 0))
			sendRestartNotification = 10;
		else
			sendRestartNotification++;
		return;
	}
}


/*
float _____readTemperatures(){

	// call sensors.requestTemperatures() to issue a global temperature 
	// request to all devices on the bus
	Serial.print("Requesting temperatures...");
	sensors.requestTemperatures(); // Send the command to get temperatures
	Serial.println("DONE");
	// After we got the temperatures, we can print them here.
	// We use the function ByIndex, and as an example get the temperature from the first sensor only.

	for (int i = 0; i < sensorCount; i++) {

		Serial.print("Temperature for the device ");
		Serial.print(sensorname[i]);
		Serial.print(" is: ");
		float dallasTemperature = sensors.getTempC(sensorAddr[i]);
		Serial.println(dallasTemperature);

		sensorTemperatures[i] = dallasTemperature;
		avTemp[i][avTempCounter] = dallasTemperature;

		sensorAvTemperatures[i] = getAverageTemperature(i);

	}

	avTempCounter++;
	if (avTempCounter >= avTempsize)
		avTempCounter = 0;

	return sensorTemperatures[0];
}
*/

/*
float getAverageTemperature(int n) {

	float average = 0;

	for (int i = 0; i < avTempsize; i++) {
		average += avTemp[n][i];
	}
	average = average / (avTempsize);

	return average;
}
*/

void showMain(boolean isPost, char* param)
{
	Serial.println(F("showMain "));

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>Webduino</title><body>\r\n");
	// comandi
	data += F("\n<font color='#53669E' face='Verdana' size='2'><b>Webduino</b></font>\r\n<table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// power
	data += F("<tr><td>Power ");
	if (programSettings.currentStatus == Program::STATUS_DISABLED) {
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
	// status
	char buffer[100];
	sprintf(buffer, "<tr><td>Status: </td><td>%s</td></tr>", statusStr[programSettings.currentStatus]);
	data += String(buffer);
	// rele
	data += F("<tr><td>Rele </td><td>");
	if (programSettings.releStatus) {
		data += F("on - ");

		time_t onStatusDuration = (millis() - programSettings.programStartTime) / 1000;
		time_t remainingTime = (programSettings.programDuration / 1000 - onStatusDuration);

		time_t hr = remainingTime / 3600L;
		time_t mn = (remainingTime - hr * 3600L) / 60L;
		time_t sec = (remainingTime - hr * 3600L) % 60L;
		sprintf(buffer, " remaining:(%2d) ", remainingTime);
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
		sprintf(buffer, " acceso da:(%2d) ", onStatusDuration);
		data += String(buffer);
		sprintf(buffer, "%02d:", hr);
		data += String(buffer);
		sprintf(buffer, "%02d:", mn);
		data += String(buffer);
		sprintf(buffer, "%02d", sec);
		data += String(buffer);

		data += F("</td></tr>");

	}
	else {
		data += F("off</td></tr>");
	}

	// program
	data += F("<tr><td>program </td><td>");
	if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE) {
		data += F("program ");
		data += activeProgram;
		data += F("timerange ");
		data += activeTimerange;

		sprintf(buffer, " Target: %d.%02d", (int)targetTemperature, (int)(targetTemperature * 100.0) % 100);
		data += String(buffer);
		data += F(" Sensor: ");
		if (temperatureSensor == sensor_local) {
			data += F("Local");
		}
		else if (temperatureSensor == sensor_remote) {
			data += F(" Remote");
		}
		data += F("</td></tr>");
	}
	else if (programSettings.currentStatus == Program::STATUS_MANUAL) {
		data += F("manual program");
		data += F("</td></tr>");
	}
	// Manual
	data += F("<tr><td>Manual ");
	if (programSettings.currentStatus == Program::STATUS_MANUAL) {

		sprintf(buffer, "ON</td><td>Target: %d.%02d<BR>", (int)targetTemperature, (int)(targetTemperature * 100.0) % 100);
		data += String(buffer);

		if (temperatureSensor == sensor_local) {
			data += F("Sensor: Local");
		}
		else if (temperatureSensor == sensor_remote) {
			data += F("Sensor: Remote");
		}
		data += F("<form action='/rele' method='POST'>");
		data += F("<input type='hidden' name='manual' value='1'>");
		data += F("<input type='hidden' name='status' value='0'>");
		data += F("<input type='submit' value='stop'></form>");
	}
	else if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE || programSettings.currentStatus == Program::STATUS_IDLE) {
		data += F("OFF</td><td><form action='/rele' method='POST'><input type='hidden' name='status' value='1'>");
		data += F("Minutes:<input type='num' name='duration' value='");

		data += 30;
		data += F("' size='5' maxlength='5'><BR>");
		data += F("<input type='hidden' name='manual' value='1'>");
		data += F("Target:<input type='number' name='target' value='22.0' step='0.10' ><BR>"); 
		data += F("Sensor:<input type='radio' name='sensor' value='0' checked>Local<input type='radio' name='sensor' value='1'>Remote<BR>");
		data += F("<input type='submit' value='start'></form>");
	}
	else {
		data += F("</td><td>--");
	}
	data += F("</td></tr>");
	// temperature
	sprintf(buffer, "<tr><td>Local Temperature: </td><td>%d.%02d</td></tr>", (int)settings.localTemperature, (int)(settings.localTemperature * 100.0) % 100);
	data += String(buffer);
	sprintf(buffer, "<tr><td>Local Average Temperature: </td><td>%d.%02d</td></tr>", (int)settings.localAvTemperature, (int)(settings.localAvTemperature * 100.0) % 100);
	data += String(buffer);
	// remote temperature
	sprintf(buffer, "<tr><td>Remote Temperature: </td><td>%d.%02d</td></tr>", (int)remoteTemperature, (int)(remoteTemperature * 100.0) % 100);
	data += String(buffer);
	// program update
	sprintf(buffer, "<tr><td>Last program or temperature update: </td><td>%d</td></tr>", (millis() - last_RemoteSensor));
	data += String(buffer);
	// sendbutton
	data += F("<tr><td>WOL</td><td><form action='/wol' method='POST'><input type='submit' value='send'></form></td><tr>");
	data += F("</table><font color='#53669E' face='Verdana' size='2'><b>Impostazioni </b></font>\r\n<form action='/chstt' method='POST'><table width='80%' border='1'><colgroup bgcolor='#B6C4E9' width='20%' align='left'></colgroup><colgroup bgcolor='#FFFFFF' width='30%' align='left'></colgroup>");
	// id
	data += F("<tr><td>ID</td><td><input type='num' name='id' value='");
	data += settings.id;
	data += F("' size='2' maxlength='2'> </td></tr>");
	// local port
	data += F("<tr><td>Local port</td><td><input type='num' name='localport");
	data += F("' value='");
	data += settings.localPort;
	data += F("' size='4' maxlength='4'> </td></tr>");
	// board name
	data += F("<tr><td>Board name</td><td><input type='num' name='boardname' value='");
	data += String(settings.boardname);
	data += F("' size='");
	data += String(settings.boardnamelen - 1);
	data += F("' maxlength='");
	data += String(settings.boardnamelen - 1);
	data += F("'> </td></tr>");
	// ssid
	data += F("<tr><td>SSID</td><td><input type='num' name='ssid");
	data += F("' value='");
	data += String(settings.networkSSID);
	data += F("' size='32' maxlength='32'> </td></tr>");
	// password
	data += F("<tr><td>password</td><td><input type='num' name='password");
	data += F("' value='");
	data += String(settings.networkPassword);
	data += F("' size='96' maxlength='96'> </td></tr>");
	// server name
	data += F("<tr><td>Server name</td><td><input type='num' name='servername' value='");
	data += String(settings.servername);
	data += F("' size='");
	data += (settings.servernamelen - 1);
	data += F("' maxlength='");
	data += (settings.servernamelen - 1);
	data += F("'> </td></tr>");
	// local port
	data += F("<tr><td>Server port</td><td><input type='num' name='serverport");
	data += F("' value='");
	data += (settings.serverPort);
	data += F("' size='4' maxlength='4'> </td></tr>");
	data += F("</table><input type='submit' value='save'/></form>");

	sprintf(buffer, "%s", Versione);
	data += String(buffer);

	data += F("</body></html>");
	client.println(data);

	delay(100);
	client.stop();
}

void showChangeSettings(char* GETparam) {

	//Serial.println(F("showChangeSettings "));
	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	char name[] = "mac0";
	for (byte i = 0; i < sizeof(mac); i++) {
		name[3] = i + 48;
		val = parsePostdata(databuff, name, posdata);
		unsigned long x;
		x = strtoul(posdata, 0, 16);
		mac[i] = x;
	}
	// localport
	val = parsePostdata(databuff, "localport", posdata);
	settings.localPort = val;
	Serial.print("localPort=");
	Serial.println(settings.localPort);
	// ssid
	val = parsePostdata(databuff, "ssid", posdata);
	memccpy_P(settings.networkSSID, posdata, '\0', sizeof(settings.networkSSID));
	Serial.print("networkSSID=");
	Serial.println(settings.networkSSID);
	// password
	val = parsePostdata(databuff, "password", posdata);
	memccpy_P(settings.networkPassword, posdata, '\0', sizeof(settings.networkPassword));
	Serial.print("networkPassword=");
	// server name
	val = parsePostdata(databuff, "servername", posdata);
	memccpy_P(settings.servername, posdata, '\0', settings.servernamelen);
	Serial.print("servername ");
	Serial.println(settings.servername);
	// server port
	val = parsePostdata(databuff, "serverport", posdata);
	Serial.print("server port ");
	Serial.println(val);
	settings.serverPort = val;
	// id
	val = parsePostdata(databuff, "id", posdata);
	//Serial.print("server port ");
	//Serial.println(val);
	settings.id = val;
	// board name
	val = parsePostdata(databuff, "boardname", posdata);
	memccpy_P(settings.boardname, posdata, '\0', settings.boardnamelen);
	Serial.print("boardname=");
	Serial.println(settings.boardname);

	String data;
	data += "";
	data += F("HTTP/1.1 200 OK\r\nContent-Type: text/html\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='0; url=/main'><title>Timer</title></head><body></body></html>");
	data += F("</body></html>");
	client.println(data);
	//client.stop();

	writeEPROM();
	client.stop();
}






bool sendNotification(int type, int value)
{
	/*Serial.println(F("sendNotification "));

	if (!netwokStarted) return false;

	char   buffer[100];
	sprintf(buffer, "{\"id\":%d,\"type\":%d,\"value\":%d}", id, type, value);

	return post(servername, serverPort, "/webduino/notification", buffer);*/

	return true;
}





/*

bool post(char* host, int port, char* path, char* param, int len, String *result)
{
	// Use WiFiClient class to create TCP connections
	WiFiClient postClient;

	String data;
	Serial.println(F("SEND POST"));
	Serial.println(host);
	Serial.println(port);
	Serial.println(path);
	Serial.println(param);
	
	data = "";
	if (client.connect(host, port)) {
		//Serial.println("connected");

		data += "POST ";
		data += path;
		data += " HTTP/1.1\r\nHost: ";
		data += host;
		data += "\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		data += "Connection: close\r\n";
		data += "Content-Length: ";
		data += len;
		data += "\r\n\r\n";
		data += param;
		data += "\r\n";

		client.print(data);
		//Serial.println(F("post data sent"));
		//Serial.println(data);

		delay(1000);

		// Read all the lines of the reply from server and print them to Serial
		while (client.available()){
			//Serial.println("client.available");
			*result = client.readStringUntil('\r');
			//Serial.print(line);
		}
		//Serial.println();
		//Serial.println("received answer - closing connection");
		//delay(1000);

	}
	else {
		//Serial.println(F("-NON CONNESSO-"));
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	return true;
}
*/

/*void unknownPage(const char *page)
{
//sendHeader("unknown");
client.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>Webduino</title><body>Unknown</body></html>"));
}*/

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

	Serial.print(F("getPostdata "));

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
		Serial.println(F("maxposdat to long"));
	}

	int i = 0;
	while (i < datalen && i < maxposdata) {
		data[i] = client.read();
		//Serial.print(data[i]); // ailitare questa riga per vedere il contenuto della post
		delay(2);
		i++;
	}

	Serial.println("");
	Serial.print("datalen ");
	Serial.print(datalen);
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

void getStatus(char* GETparam)
{
	Serial.print(F("getStatus start"));
	Serial.print(F("+++>Memory free: "));
	Serial.println(memoryFree());

	client.print(F("HTTP/1.0 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nPragma: no-cache\r\n\r\n"));

	client.print(F("{\"id\":"));
	client.print(settings.id, DEC);

	client.print(F(",\"temperature\":"));
	float temp = settings.localTemperature;
	client.print(temp, DEC);

	client.print(F(",\"avtemperature\":"));
	temp = settings.localAvTemperature;
	client.print(temp, DEC);

	client.print(F(",\"remotetemperature\":"));
	temp = remoteTemperature;
	client.print(temp, DEC);

	char buf[50];
	sprintf(buf, ",\"status\":\"%s\"", statusStr[programSettings.currentStatus]);
	client.print(buf);

	client.print(F(",\"relestatus\":"));
	if (programSettings.releStatus)
		client.print(F("true"));
	else
		client.print(F("false"));

	if (programSettings.currentStatus == Program::STATUS_PROGRAMACTIVE || programSettings.currentStatus == Program::STATUS_MANUAL) {

		client.print(F(",\"duration\":"));
		client.print(programSettings.programDuration, DEC);

		int remainingTime = programSettings.programDuration - (millis() - programSettings.programStartTime);
		client.print(F(",\"remaining\":"));
		client.print(remainingTime, DEC);

		client.print(F(",\"localsensor\":"));
		if (temperatureSensor == sensor_local)
			client.print(F("true"));
		else
			client.print(F("false"));

		client.print(F(",\"target\":"));
		client.print(targetTemperature);

		client.print(F(",\"program\":"));
		client.print(activeProgram, DEC);

		client.print(F(",\"timerange\":"));
		client.print(activeTimerange, DEC);

	}
	client.print(F("}"));
	delay(300);
	client.stop();

	Serial.print(F("getStatus end"));
	Serial.print(F("<---Memory free: setup "));
	Serial.println(memoryFree());

}

void setRemoteTemperature(char* GETparam) {

	Serial.println(F("setRemoteTemperature "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	// status
	parsePostdata(databuff, "temperature", posdata);
	String str = "";
	str += posdata;
	remoteTemperature = str.toFloat();

	last_RemoteSensor = millis();

	/*Serial.print(F("remoteTemperature="));
	Serial.println(remoteTemperature);

	client.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/html"));

	client.println(F("\n\n<html><head><meta HTTP-EQUIV='REFRESH' content='2; url=/main?msg=2'><title>WOL</title></head><body>"));

	char   buffer[50];
	sprintf(buffer, "remote=%s", posdata);
	client.print(buffer);

	client.println(F("</body></html>"));*/
	getStatus(NULL);

}

void showPower(char* GETparam) {

	Serial.println(F("showPower "));

	getPostdata(databuff, maxposdataChangeSetting);
	char posdata[maxposdata];

	int val;
	// status
	val = parsePostdata(databuff, "status", posdata);
	Serial.print(F("status="));
	Serial.println(val, DEC);


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

}
