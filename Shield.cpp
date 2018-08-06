#include "Shield.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "Logger.h"
#include "Command.h"
#include "ESP8266Webduino.h"
#include "JSONArray.h"
#ifdef ESP8266
#include <Adafruit_ST7735.h>
#endif
#include "SensorFactory.h"

extern void writeEPROM();
extern void resetEPROM();
extern void resetWiFiManagerSettings();

extern bool mqtt_publish(String topic, String message);
//extern bool _mqtt_publish(char* topic, char* payload);

Logger Shield::logger;
String Shield::tag = "Shield";

String Shield::lastRestartDate = "";
String Shield::swVersion = "1.55";
int Shield::id = 0; //// inizializzato a zero perch� viene impostato dalla chiamata a registershield

int Shield::localPort = 80;
String Shield::serverName = "192.168.1.33";
String Shield::mqttServer = "192.168.1.33";
int Shield::mqttPort = 1883;
int Shield::serverPort = 8080;
String Shield::shieldName = "shieldName";

unsigned char Shield::MAC_array[6];
char Shield::MAC_char[18];
bool Shield::mqttMode = true;// true;
bool Shield::configMode = false;// true;
bool Shield::resetSettings = false;// true;
String Shield::powerStatus = "on"; // da aggiungere

Shield::Shield()
{
	lastCheckHealth = 0;
}

Shield::~Shield()
{
	// E' inutile perchè non lo chiamerà mai
	sensorList.clearAll();
}

void Shield::init() {
	//tftDisplay.init();
	//display.init();
	status = "restart";
	shieldEvent = "";

	espDisplay.init(D3, D4);
}

void Shield::clearAllSensors() {

	sensorList.clearAll();
}

Sensor* Shield::getSensorFromAddress(String addr) {
	for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (sensor->address == addr)
			return sensor;
	}
	return nullptr;
}

Sensor* Shield::getSensorFromId(int id) { /// sidsogna aggiungere anche richerca nei child
	logger.print(tag, "\n\t >>getSensorFromId " + String(id));
	logger.print(tag, "\n\t sensorList.count= " + String(sensorList.count));
	for (int i = 0; i < sensorList.count; i++) {
		logger.print(tag, "\n\t i= " + String(i));
		Sensor* sensor = (Sensor*)sensorList.get(i);
		logger.print(tag, "\n\t sensor->sensorid= " + String(sensor->sensorid));
		logger.print(tag, " sensor->sensorname= " + sensor->sensorname);
		if (sensor->sensorid == id) {
			logger.print(tag, "\n\t sensor found " + sensor->sensorname);
			logger.print(tag, "\n\t <<getSensorFromId");
			return sensor;
		}
		logger.print(tag, "\n\t childsensors.length()= " + String(sensor->childsensors.length()));
		for (int k = 0; k < sensor->childsensors.length(); k++) {
			logger.print(tag, "\n\t child k= " + String(k));
			Sensor* child = (Sensor*)sensor->childsensors.get(k);
			logger.print(tag, "\n\t childsensors= " + String(child->sensorid));
			logger.print(tag, " name= " + String(child->sensorname));
			if (child->sensorid == id) {
				logger.print(tag, "\n\t sensor child found " + child->sensorname);
				logger.print(tag, "\n\t <<getSensorFromId");
				return (Sensor*)child;
			}
		}
	}
	logger.println(tag, "\n\t sensor not found");
	logger.print(tag, "\n\t <<getSensorFromId");
	return nullptr;
}


void Shield::drawStatus() {
	espDisplay.drawString(0, 0, status);
}

void Shield::drawEvent() {

	espDisplay.drawString(0, 10, shieldEvent);
}

void Shield::drawSWVersion() {

	String txt = logger.getStrDate();
	espDisplay.drawString(80, 0, "Ver." + swVersion);
}

void Shield::drawDateTime() {

	String txt = logger.getStrDate();
	espDisplay.drawString(0, 20, txt);
}

void Shield::drawSensorsStatus() {

	int lines = 0;
	for (int i = 0; i < sensorList.count; i++) {

		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (!sensor->enabled)
			continue;
		espDisplay.drawString(0, (lines++ + 3) * 10, String(sensor->sensorname) + ": " + String(sensor->getStatusText()));
		for (int k = 0; k < sensor->childsensors./*count*/length(); k++) {
			Sensor* child = (Sensor*)sensor->childsensors.get(k);
			espDisplay.drawString(0, (lines++ + 3) * 10, String(child->sensorname) + ": " + String(child->getStatusText()));
		}
	}
}



void Shield::drawString(int x, int y, String txt, int size, int color) {

	//tftDisplay.drawString(x, y, txt, 1, ST7735_WHITE);
	//tftDisplay.drawString(int row, int col, String txt, int size, int color);
}

/*Just as a heads up, to use the "drawXBitmap" function, I also had
to redefine the array from "static unsigned char" to "static const uint8_t PROGMEM"
in my image file.If I just used the original file, then garbage would appear on my 32x32 gird.
I think the array has to be defined as "PROGMEM" because the "drawXBitmap" function uses the
"pgm_read_byte" function, which reads a byte from program memory.I also had to include
the line "#include <avr/pgmspace.h>".After that, everything worked fine for me.*/

//#include <avr/pgmspace.h>
#define temperature_width 32
#define temperature_height 32
static const uint8_t PROGMEM temperature_bits[] = {
	0x00, 0x80, 0x01, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x70, 0x0e, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x08, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x18, 0x10, 0x00, 0x00, 0x78, 0x10, 0x00, 0x00, 0x18, 0x10, 0x00,
	0x00, 0x18, 0x10, 0x00, 0x00, 0x78, 0x10, 0x00, 0x00, 0x18, 0x10, 0x00,
	0x00, 0x08, 0x10, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00,
	0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00,
	0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xcc, 0x33, 0x00,
	0x00, 0xc4, 0x23, 0x00, 0x00, 0xe6, 0x67, 0x00, 0x00, 0xe2, 0x67, 0x00,
	0x00, 0xe6, 0x67, 0x00, 0x00, 0xe6, 0x67, 0x00, 0x00, 0xe6, 0x67, 0x00,
	0x00, 0xce, 0x71, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x3c, 0x3c, 0x00,
	0x00, 0xf0, 0x0f, 0x00, 0x00, 0xc0, 0x03, 0x00 };

void Shield::clearScreen() {
	//tftDisplay.clear();

	//tftDisplay.drawXBitmap(90, 50, temperature_bits, temperature_width, temperature_height, 0xF800/*ST3577_RED*/);
}

bool Shield::onUpdateSensorListCommand(JSON& json) {

	logger.print(tag, "\n\t >>onUpdateSensorListCommand");
	sensorList.clearAll();

	if (json.has("sensors")) {

		String str = json.jsonGetArrayString("sensors");
		logger.print(tag, "\n\t str=" + str);
		JSONArray jArray(str);
		String item = jArray.getFirst();

		SensorFactory factory;
		clearAllSensors(); // serve per inizializzare
		int i = 0;
		while (!item.equals("")) {

			logger.print(tag, "\n\n\t SENSOR: #" + String(i++));
			logger.print(tag, " =" + item);

			JSONObject jObject(item);
			logger.print(tag, "json=" + jObject.toString());

			Sensor* sensor = factory.createSensor(&jObject);
			if (sensor != nullptr) {
				//addSensor(sensor);
				sensorList.add(sensor);
			}
			item = jArray.getNext();
		}
		sensorList.show();
		writeEPROM();
	}
	logger.print(tag, "\n\t <<onUpdateSensorListCommand");
	return true;
}


bool Shield::receiveCommand(String jsonStr) {

	logger.print(tag, "\n\t >>receiveCommand");
	logger.printFreeMem(tag, "++receiveCommand");
	logger.print(tag, "\n\t json=" + jsonStr);

	JSON json(jsonStr);
	if (json.has("command")) {
		bool result = false;
		String command = json.jsonGetString("command");
		logger.print(tag, "\n\t command=" + command);
		setEvent("<-received command " + command);
		if (command.equals("updatesensorlist")) { // ???
			result = onUpdateSensorListCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals("shieldsettings")) { // risposta a loadsettting
			result = onShieldSettingsCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals("power")) { // ??
			result = onPowerCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals("reset")) { // che differenza c'è tra reboot e reset?
			result = onResetCommand(json);
			result = onRebootCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals("reboot")) { // che differenza c'è tra reboot e reset?
			result = onRebootCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals("checkhealth")) {
			logger.printFreeMem(tag, "++checkhealth");

			// uuid
			String uuid = "";
			if (json.has("uuid")) {
				uuid = json.jsonGetString("uuid");
				logger.print(tag, "\n\t uuid=" + uuid);
			}
			else {
				logger.print(tag, "\n\t uuid NOT FOUND!");
				return false;
			}

			lastCheckHealth = millis();
			String topic = "toServer/response/" + uuid + "/success";
			bool res = mqtt_publish(topic, getJson());

			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return res;
		}
		else if (json.has("actuatorid") && json.has("uuid")) {	// se c'è il campo actuatorid

			logger.print(tag, "\n\treceived actuator command");
			logger.printFreeMem(tag, "++actuatorcommand");				// allora è un comando per un sensore			
			int id = json.jsonGetInt("actuatorid");
			logger.print(tag, "\n\t actuatorid=" + String(id));
			String uuid = json.jsonGetString("uuid");
			Sensor* sensor = getSensorFromId(id);
			if (sensor != nullptr) {
				logger.printFreeMem(tag, "++receiveCommand");
				setEvent("<-received sensor command " + command);
				return sensor->receiveCommand(command, id, uuid, jsonStr);
			}
			else {
				logger.print(tag, "\n\t sensor not found");
				logger.print(tag, "\n\t <<receiveCommand result");
			}
		}
	}
	logger.printFreeMem(tag, "--receiveCommand");
	logger.print(tag, "\n\t <<receiveCommand");
	return false;
}

bool Shield::onShieldSettingsCommand(JSON& json)
{
	logger.print(tag, "\n\t>>onShieldSettingsCommand");

	if (json.has("localport")) {
		int localPortr = json.jsonGetInt("localport");
		logger.print(tag, "\n\t localport=" + localPort);
		setLocalPort(localPort);
	}
	if (json.has("shieldname")) {
		String name = json.jsonGetString("shieldname");
		logger.print(tag, "\n\t shieldname=" + name);
		setShieldName(name);
	}
	if (json.has("servername")) {
		String name = json.jsonGetString("servername");
		logger.print(tag, "\n\t servername=" + name);
		setServerName(name);
	}
	if (json.has("serverport")) {
		int serverPort = json.jsonGetInt("serverport");
		logger.print(tag, "\n\t serverport=" + serverPort);
		setServerPort(serverPort);
	}
	writeEPROM();

	logger.print(tag, "\n\t <<onShieldSettingsCommand");
	return true;
}

bool Shield::onRebootCommand(JSON& json)
{
	logger.print(tag, "\n\t>> onRebootCommand");
	ESP.restart();
	return true;
}

bool Shield::onResetCommand(JSON& json)
{
	logger.print(tag, "\n\t>> onResetCommand");
	resetWiFiManagerSettings();
	resetEPROM(); // scrive 0 nel primo settore della EPROM per azzerare tutto

	return true;
}

bool Shield::onPowerCommand(JSON& json)
{
	logger.print(tag, "\n\t>> onPowerCommand");
	bool res = false;

	if (json.has("status")) {
		String status = json.jsonGetString("status");
		logger.print(tag, "\n\t status=" + status);

		if (status.equals("on")) {
			setPowerStatus(status);
			res = true;
		}
		else if (status.equals("off")) {
			setPowerStatus(status);
			res = true;
		}
	}
	logger.print(tag, "\n\t<< onPowerCommand");
	return true;
}

String Shield::getJson() {

	logger.print(tag, "\n\t >>Shield::getJson");

	// andrebbe trasformato in JSONObject
	String str = "{";

	str += "\"MAC\":\"" + String(MAC_char) + "\"";
	str += ",\"swversion\":\"" + swVersion + "\"";
	str += ",\"lastreboot\":\"" + lastRestartDate + "\"";
	str += ",\"lastcheckhealth\":\"" + String(lastCheckHealth) + "\"";
	str += ",\"localIP\":\"" + localIP + "\"";
	str += ",\"localPort\":\"" + String(Shield::getLocalPort()) + "\"";
	str += "}";

	logger.print(tag, "\n\tstr=" + str + "\n");
	logger.print(tag, "\n\t <<Shield::getJson");
	return str;
}

bool Shield::_getSensorsStatusJson(char *payload) {

	//Serial.println(ESP.getFreeHeap());

	//logger.print(tag, "\n\n");
	logger.println(tag, F("\n\t >>_getSensorsStatusJson"));

	sensorList.show();

	sprintf(payload, "{\"shieldid\":%d,\"swversion\":\"%s\",\"sensors\":[", id, Shield::swVersion.c_str());

	int payloadcount = strlen(payload);

	for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = (Sensor*)sensorList.get(i);

		if (i != 0)
			payload[payloadcount++] = ',';
		String str = sensor->getJSON();
		for (int i = 0; i < str.length(); i++) {
			payload[payloadcount++] = str.charAt(i);
		}
	}

	payload[payloadcount++] = ']';
	payload[payloadcount++] = '}';
	payload[payloadcount++] = '\0';
	Serial.print("payload=");
	Serial.print(payload);
	Serial.print("--Endpayload");
	Serial.println(payloadcount, DEC);
	logger.println(tag, "<<_getSensorsStatusJson = ");
	return payload;
}

String Shield::getSensorsStatusJson() {

	//logger.print(tag, "\n\n");
	logger.print(tag, F("\n\t >>getSensorsStatusJson"));
	sensorList.show();

	String json = "{";
	json += "\"shieldid\":" + String(id);// shieldid
	json += ",\"swversion\":\"" + Shield::swVersion + "\"";// shieldid
	//json += ",\"power\":\"" + Shield::powerStatus + "\"";// shieldid

	json += ",\"sensors\":[";

	for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (i != 0)
			json += ",";
		json += sensor->getJSON();
		logger.println(tag, "\n\n\t json length=" + String(json.length()));
	}
	json += "]";
	json += "}";

	logger.println(tag, "<<getSensorsStatusJson = ");
	return json;
}

String Shield::getSettingsJson() { // usata per le impostazioni da jscript pages
	String json = "{";

	json += "\"localport\":" + String(localPort);
	json += ",\"shieldname\":\"" + shieldName + "\"";
	//json += ",\"ssid\":\"" + networkSSID + "\"";
	//json += ",\"password\":\"" + networkPassword + "\"";
	json += ",\"servername\":\"" + serverName + "\"";
	json += ",\"serverport\":" + String(serverPort);
	json += ",\"mqttserver\":\"" + mqttServer + "\"";
	json += ",\"mqttport\":" + String(mqttPort);
	json += ",\"localip\":\"" + String(localIP) + "\"";
	json += ",\"macaddress\":\"" + String(MAC_char) + "\"";
	json += ",\"shieldid\":" + String(id);
	json += ",\"datetime\":\"" + Logger::getStrDate() + "\"";
	json += ",\"power\":\"" + powerStatus + "\"";
	json += ",\"lastrestart\":\"" + lastRestartDate + "\"";
	json += ",\"heap\":\"" + String(ESP.getFreeHeap()) + "\"";
	json += ",\"swversion\":\"" + swVersion + "\"";

	json += "}";
	return json;
}

void Shield::setStatus(String txt) {
	if (status.equals(txt))
		return;
	status = txt;
	invalidateDisplay();
}

void Shield::setEvent(String txt) {
	if (shieldEvent.equals(txt))
		return;
	shieldEvent = txt;
	invalidateDisplay();
}

void Shield::invalidateDisplay() {
	espDisplay.clear();

	drawStatus();
	drawSWVersion();
	drawEvent();
	drawDateTime();
	drawSensorsStatus();
	espDisplay.update();
}

void Shield::checkStatus()
{
	checkSensorsStatus();

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastTimeUpdate;

	if (timeDiff > 1000) {
		lastTimeUpdate = currMillis;

		invalidateDisplay();
	}


	/*display.clear();
	uint32_t getVcc = ESP.getVcc();// / 1024;
	String voltage = "Vcc " + String(getVcc) + "V";

	display.drawString(0, 0, Logger::getStrTimeDate() + " ", ArialMT_Plain_16);
	display.drawString(20, 16, Logger::getStrDayDate() + " ", ArialMT_Plain_10);
	display.drawString(0, 26, hearterActuator.getStatusName() + " " + String(hearterActuator.getReleStatus()) + voltage, ArialMT_Plain_10);
	for (int i = 0; i < sensorList.count; i++) {
		DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);
		display.drawString(0, 16 + 10 + 10 + 10*(i), pSensor->sensorname + " " + pSensor->getTemperature(), ArialMT_Plain_10);
	}
	display.update();*/


	String date = Logger::getStrDate();
	//logger.println(tag, "dater:" + date);
	//if (!date.equals(oldDate)) {
		//tftDisplay.drawString(0, 0, Logger::getStrTimeDate(), 1, ST7735_WHITE);
		//tftDisplay.drawString(20, 16, Logger::getStrDayDate() + " ", 1, ST7735_WHITE);
	//}
	//oldDate = date;

	//tftDisplay.clear();
	//int textWidth = 5;
	//int textHeight = 8;

	//tftDisplay.drawString(0, 0, Logger::getStrDayDate() + " ", 1, ST7735_WHITE);
	//tftDisplay.drawString(0, textHeight, Logger::getStrTimeDate() + " ", 2, ST7735_WHITE);

	//tftDisplay.drawString(0, textHeight*(2 + 1), phearterActuator->getStatusName(), 2, ST7735_WHITE);

	/*for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (!sensor->type.equals("temperature"))
			continue;
	}*/
	//display.update();
}

void Shield::checkSensorsStatus()
{
	//logger.println(tag, ">>checkSensorsStatus" + String(sensorList.count));

	bool sensorStatusChanged = false;

	for (int i = 0; i < sensorList.count; i++) {

		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (!sensor->enabled)
			continue;
		//logger.print(tag, "\n\t sensor->name: " + String(sensor->sensorname));
		bool res = sensor->checkStatusChange();

		unsigned long currMillis = millis();
		unsigned long timeDiff = currMillis - sensor->lastUpdateStatus;

		// Invia il comando se è cambiato lo stato del sensore
		// oppure se l’ultimo invio è fallito 
		// oppure se è passato il timeout dall’ultimo invio
		if (res || /*sensor->lastUpdateStatusFailed ||*/ timeDiff > sensor->updateStatus_interval) {
			// eliminato reinvio su LasUpdateStatusFailed perchè va in loop infinito
			setEvent("SENSOR STATUS CHANGED");

			logger.println(tag, ">>SEND SENSOR STATUS");
			logger.print(tag, "\n\t sensor->name: " + String(sensor->sensorname));
			if (res)
				logger.print(tag, "\n\t status changed" + String(sensor->status));
			if (timeDiff > sensor->updateStatus_interval)
				logger.print(tag, "\n\t updateStatus_interval timeout");

			//sensorStatusChanged = true;
			sensor->lastUpdateStatus = millis();

			Command command;
			bool res = command.sendSensorStatus(sensor->getJSON());
			sensor->lastUpdateStatusFailed = !res;
			logger.println(tag, "<<SEND SENSOR STATUS");
		}
	}

	/*if (sensorStatusChanged*) {
		Command command;
		logger.println(tag, "\t >>checkSensorsStatus() SEND SENSOR STATUS UPDATE\n");
		char payload[1000];
		bool res  = _getSensorsStatusJson(payload);
		//logger.println(tag, "\n json=" + json + "\n");
		command._sendSensorsStatus(payload);

		logger.println(tag, "\t <<checkSensorsStatus() SEND SENSOR STATUS UPDATE\n");
	}*/
}

void Shield::addSensor(Sensor* pSensor) {

	logger.print(tag, "\n\t >>addSensor " + pSensor->sensorname);
	sensorList.add((Sensor*)pSensor);
	//sensorList.show();
	logger.print(tag, "\n\t <<addSensor");
}
