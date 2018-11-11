#include "Shield.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "Logger.h"
#include "Command.h"
#include "ESP8266Webduino.h"
#ifdef ESP8266
#include <Adafruit_ST7735.h>
#endif
#include "SensorFactory.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "MQTTMessage.h"

extern void resetWiFiManagerSettings();
extern bool mqtt_publish(MQTTMessage mqttmessage);
extern void reboot(String reason);

Logger Shield::logger;
String Shield::tag = "Shield";

Shield::Shield()
{
	lastRestartDate = "";
	swVersion = "1.84";

	id = 0; //// inizializzato a zero perch� viene impostato dalla chiamata a registershield
	localPort = 80;
	serverName = "giacomohome.ddns.net";
	mqttServer = "giacomohome.ddns.net";
	mqttPort = 1883;
	serverPort = 8080;
	shieldName = "shieldName";
	mqttUser = "";
	mqttPassword = "";
	mqttMode = true;// true;
	oleddisplay = false;
	nexiondisplay = false;

	configMode = false;// true;
	resetSettings = false;// true;
	powerStatus = "on"; // da aggiungere
	lastCheckHealth = 0;
	rebootreason = "unknown";
}

Shield::~Shield()
{
	sensors.clear();
}

void Shield::writeConfig() {

	logger.println(tag, F(">>writeConfig"));
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["http_server"] = getServerName();
	json["http_port"] = getServerPort();
	json["mqtt_server"] = getMQTTServer();
	json["mqtt_port"] = getMQTTPort();
	json["mqtt_user"] = getMQTTUser();
	json["mqtt_password"] = getMQTTPassword();
	json["resetsettings"] = getResetSettings();
	json["shieldid"] = getShieldId();

	File configFile = SPIFFS.open(F("/config.json"), "w");
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

void Shield::readConfig() {

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
						setServerName(str);
					}
					if (json.containsKey("http_port")) {
						logger.print(tag, F("\n\t http_port: "));
						String str = json["http_port"];
						logger.print(tag, str);
						setServerPort(json["http_port"]);
					}
					if (json.containsKey("mqtt_server")) {
						logger.print(tag, "\n\t mqtt_server: ");
						String str = json["mqtt_server"];
						logger.print(tag, str);
						setMQTTServer(json["mqtt_server"]);
					}
					if (json.containsKey("mqtt_port")) {
						logger.print(tag, F("\n\t mqtt_port: "));
						String str = json["mqtt_port"];
						logger.print(tag, str);
						setMQTTPort(json["mqtt_port"]);
					}
					if (json.containsKey("mqtt_user")) {
						logger.print(tag, F("\n\t mqtt_user: "));
						String str = json["mqtt_user"];
						logger.print(tag, str);
						setMQTTUser(json["mqtt_user"]);
					}
					if (json.containsKey("mqtt_password")) {
						logger.print(tag, F("\n\t mqtt_password: "));
						String str = json["mqtt_password"];
						logger.print(tag, str);
						setMQTTPassword(json["mqtt_password"]);
					}
					if (json.containsKey("shieldid")) {
						logger.print(tag, F("\n\t shieldid: "));
						String str = json["shieldid"];
						logger.print(tag, str);
						setShieldId(json["shieldid"]);
					}
					if (json.containsKey("resetsettings")) {
						logger.print(tag, F("\n\t resetsettings: "));
						bool enabled = json["resetsettings"];
						logger.print(tag, Logger::boolToString(enabled));
						setResetSettings(json["resetsettings"]);
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


void Shield::init() {
	//tftDisplay.init();
	//display.init();
	status = "restart";
	shieldEvent = "";

	espDisplay.init(D3, D4);
}

void Shield::clearAllSensors() {

	//sensorList.clearAll();
	sensors.clear();
}

Sensor* Shield::getSensorFromAddress(String addr) {

	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		if (sensor->address == addr)
			return sensor;
	}
	return nullptr;
}

Sensor* Shield::getSensorFromId(int id) { /// sidsogna aggiungere anche richerca nei child
	logger.print(tag, F("\n\t >>Shield::getSensorFromId"));

	//logger.print(tag, String(F("\n\t sensors.size=")) + String(sensors.size()));
	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		//logger.print(tag, String(F("\n\t sensorid=")) + String(sensor->sensorid));
		if (sensor->sensorid == id) {
			logger.print(tag, String(F("\n\t <<>Shield::getSensorFromId - found")));
			return sensor;
		}
		//logger.print(tag, String(F("\n\t childsensors.size=")) + String(sensor->childsensors.size()));
		for (int k = 0; k < sensor->childsensors.size(); k++) {
			Sensor* child = (Sensor*)sensor->childsensors.get(k);
			logger.print(tag, String(F("\n\t childsensorid=")) + String(child->sensorid));
			if (child->sensorid == id) {
				logger.print(tag, String(F("\n\t <<>Shield::getSensorFromId - found")));
				return (Sensor*)child;
			}
		}
	}
	logger.print(tag, String(F("\n\t <<>Shield::getSensorFromId - NOT found!")));
	return nullptr;
}


void Shield::drawStatus() {
	espDisplay.drawString(0, 0, status);
}

void Shield::drawMemory() {
	espDisplay.drawString(50, 0, "(" + String(freeMemory) + ")");
}

void Shield::drawEvent() {

	espDisplay.drawString(0, 10, shieldEvent);
}

void Shield::drawSWVersion() {

	String txt = logger.getStrDate();
	espDisplay.drawString(100, 0, "v" + swVersion);
}

void Shield::drawDateTime() {

	String txt = logger.getStrDate();
	espDisplay.drawString(0, 20, txt);
}

void Shield::drawSensorsStatus() {

	int lines = 3;
	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		if (!sensor->enabled)
			continue;
		String txt = sensor->sensorname.substring(0, 5);
		espDisplay.drawString(0, lines++ * 10, txt + ": " + String(sensor->getStatusText()));
		for (int k = 0; k < sensor->childsensors.size(); k++) {
			Sensor* child = (Sensor*)sensor->childsensors.get(k);
			txt = child->sensorname.substring(0, 5);
			espDisplay.drawString(0, lines++ * 10, txt + ": " + String(child->getStatusText()));
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

void Shield::parseMessageReceived(String topic, String message) {

	logger.print(tag, F("\n\t >>parseMessageReceived"));
	String str = String(F("fromServer/shield/")) + getMACAddress();

	logger.print(tag, String(F("\n\t topic: ")) + topic);
	if (topic.equals(str + F("/settings"))) {
		logger.print(tag, F("\n\t received shield settings\n"));
		logger.print(tag, message);
		logger.print(tag, F("\n"));
		setEvent(F("<-received settings"));

		size_t size = message.length();
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(message);
		if (json.success()) {
			logger.print(tag, F("\n\t settings= \n"));
			logger.printJson(json);
		}
		else {
			logger.print(tag, F("failed to load json config"));
			return;
		}
		setRebootReason("unknown");
		if (loadSensors(json)) {
			writeSensorToFile(json);
			settingFromServerReceived = true;
			Command command;
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			getJson(json);
			String str;
			json.printTo(str);
			command.sendShieldStatus(str);
		}
	}
	else if (topic.equals(str + F("/reboot"))) {
		logger.print(tag, F("\n\t received reboot request"));
		setEvent(F("<-received reboot request"));
		reboot("reboot command");
	}
	else if (topic.equals(str + F("/resetsettings"))) {
		logger.print(tag, F("\n\t RESET SETTINGS!!!!!!!!!!!!!!!!"));
		setResetSettings(true);
		writeConfig();
		reboot("reset");
	}
	else if (topic.equals(str + F("/time"))) { // risposta a time
		logger.print(tag, F("\n\t received time"));
		setEvent(F("<-received time"));

		lastTimeRequest = millis();
		timeNeedToBeUpdated = false;
		bool timeRequestInprogress = false;

		time_t t = (time_t)atoll(message.c_str());
		logger.print(tag, F("\n\t time="));
		logger.print(tag, String(t));
		setTime(t);
	}
	else if (topic.equals(str + F("/command"))) {
		logger.print(tag, F("\n\t received command "));
		logger.print(tag, message);
		setEvent(F("<-received command"));
		receiveCommand(message);
	}

	else {
		logger.print(tag, F("\n\t PARSE NOT FOUND"));
	}
	logger.printFreeMem(tag, F("parseMessageReceived"));
	logger.print(tag, F("\n\t <<parseMessageReceived"));
}

bool Shield::receiveCommand(String jsonStr) {

	logger.print(tag, F("\n\t >>Shield::receiveCommand"));
	logger.print(tag, jsonStr);

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(jsonStr);
	if (json.success()) {
		logger.print(tag, F("\n\t command= \n"));
		logger.printJson(json);
	}
	else {
		logger.print(tag, F("\n\t failed to load json config"));
		return false;
	}

	if (json.containsKey("command")) {
		bool result = false;
		String command = json["command"];
		logger.print(tag, F("\n\t command="));
		logger.print(tag, command);
		setEvent("<-received command " + command);
		if (command.equals(F("shieldsettings"))) { // risposta a loadsettting
			result = onShieldSettingsCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals(F("checkhealth"))) {
			logger.printFreeMem(tag, F("++checkhealth"));

			// uuid
			String uuid = "";
			if (json.containsKey("uuid")) {
				String str = json["uuid"];
				uuid = str;
				logger.print(tag, F("\n\t uuid="));
				logger.print(tag, uuid);
			}
			else {
				logger.print(tag, F("\n\t uuid NOT FOUND!"));
				return false;
			}

			lastCheckHealth = millis();
			MQTTMessage mqttmessage;
			mqttmessage.topic = "toServer/response/" + uuid + "/success";
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			getJson(json);
			//String str;
			json.printTo(mqttmessage.message);
			bool res = mqtt_publish(mqttmessage);

			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return res;
		}
		else if (json.containsKey("sensorid") && json.containsKey("uuid")) {	// se c'è il campo actuatorid

			logger.print(tag, F("\n\treceived actuator command"));
			int id = json["sensorid"];
			logger.print(tag, F("\n\t sensorid="));
			logger.print(tag, String(id));
			String uuid = json["uuid"];
			Sensor* sensor = getSensorFromId(id);
			if (sensor != nullptr) {
				setEvent(F("<-received sensor command "));
				setEvent(command);
				return sensor->receiveCommand(command, id, uuid, jsonStr);
			}
			else {
				logger.print(tag, F("\n\t sensor not found"));
				logger.print(tag, F("\n\t <<receiveCommand result"));
			}
		}
	}
	logger.printFreeMem(tag, F("--receiveCommand"));
	logger.print(tag, F("\n\t <<Shield::receiveCommand"));
	return false;
}

bool Shield::onShieldSettingsCommand(JsonObject& json)
{
	logger.print(tag, F("\n\t>>onShieldSettingsCommand"));

	if (json.containsKey("localport")) {
		int localPortr = json["localport"];
		//logger.print(tag, "\n\t localport=" + localPort);
		setLocalPort(localPort);
	}
	if (json.containsKey("shieldname")) {
		String name = json["shieldname"];
		//logger.print(tag, "\n\t shieldname=" + name);
		setShieldName(name);
	}
	if (json.containsKey("servername")) {
		String name = json["servername"];
		//logger.print(tag, "\n\t servername=" + name);
		setServerName(name);
	}
	if (json.containsKey("serverport")) {
		int serverPort = json["serverport"];
		//logger.print(tag, "\n\t serverport=" + serverPort);
		setServerPort(serverPort);
	}
	logger.print(tag, F("\n\t <<onShieldSettingsCommand"));
	return true;
}

bool Shield::onPowerCommand(JsonObject& json)
{
	logger.print(tag, F("\n\t>> onPowerCommand"));
	bool res = false;

	if (json.containsKey(F("status"))) {
		String status = json["status"];
		//logger.print(tag, "\n\t status=" + status);

		if (status.equals(F("on"))) {
			setPowerStatus(status);
			res = true;
		}
		else if (status.equals(F("off"))) {
			setPowerStatus(status);
			res = true;
		}
	}
	logger.print(tag, F("\n\t<< onPowerCommand"));
	return true;
}

void Shield::getJson(JsonObject& json) {
	logger.print(tag, F("\n\t >>Shield::getJson"));
	json["MAC"] = getMACAddress();
	json["swversion"] = swVersion;
	json["lastreboot"] = lastRestartDate;
	json["lastcheckhealth"] = String(lastCheckHealth);
	json["freemem"] = String(freeMemory);
	json["localIP"] = localIP;
	json["localPort"] = String(Shield::getLocalPort());
	logger.print(tag, F("\n\t <<Shield::getJson json="));
	logger.printJson(json);
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
	drawMemory();
	drawSWVersion();
	drawEvent();
	drawDateTime();
	drawSensorsStatus();
	espDisplay.update();
}

void Shield::setFreeMem(int mem)
{
	freeMemory = mem;
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

}

void Shield::checkSensorsStatus()
{
	//logger.print(tag, F("\n\t >>checkSensorsStatus"));
	bool sensorStatusChanged = false;

	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		if (!sensor->enabled) {
			continue;
		}

		bool sendUpdate = false;
		bool res = sensor->checkStatusChange();
		if (res) {
			sendUpdate = true;
			setEvent(F("SENSOR STATUS CHANGED"));
			logger.println(tag, F("SENSOR STATUS CHANGED "));
			logger.print(tag, F("\n\t sensor "));
			logger.print(tag, sensor->sensorid);
			logger.print(tag, F("."));
			logger.print(tag, sensor->sensorname);
			logger.print(tag, F("\n\t sensor status changed - status: "));
			logger.print(tag, String(sensor->status));
		}

		unsigned long timeDiff = millis() - sensor->lastUpdateStatus;
		if (timeDiff > sensor->updateStatus_interval) {
			setEvent(F("SENSOR STATUS UPDATE TIMEOUT"));
			logger.println(tag, F("SENSOR STATUS UPDATE TIMEOUT "));
			logger.print(tag, F("\n\t sensor "));
			logger.print(tag, sensor->sensorid);
			logger.print(tag, F("."));
			logger.print(tag, sensor->sensorname);
			logger.print(tag, F("\n\t update status timeout"));
			sendUpdate = true;
		}

		if (sendUpdate) {
			Command command;
			/*DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			sensor->getJson(json);*/

			/*StaticJsonBuffer<500> jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();*/
			
			//json["date"] = logger.getStrDate();

			/*String str2;
			json.printTo(str2);
			logger.print(tag, str2);

			sensor->getJson(json);*/

			/*String str;
			json.printTo(str);
			logger.print(tag, str);*/
			String strJson = sensor->getStrJson();
			

			//bool res = command.sendSensorStatus(json);
			bool res = command.sendStrSensorStatus(strJson);
			logger.print(tag, F("\n\t SENSOR STATUS SENT - res="));
			logger.print(tag, Logger::boolToString(res));
			if (res) {
				sensor->lastUpdateStatus = millis();
			}
		}
	}
	//logger.print(tag, F("\n\t <<checkSensorsStatus"));
}

bool Shield::requestTime() {
	logger.print(tag, F("\n\t >>requestTimeFromServer"));
	setEvent(F("request server setting.."));
	timeNeedToBeUpdated = false;
	lastTimeRequest = millis();
	Command command;
	bool res = command.requestTime(getMACAddress());
	if (res)
		timeRequestInprogress = true;
	else
		timeRequestInprogress = false;
	logger.print(tag, F("\n\t <<requestTimeFromServer res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

void Shield::checkTimeUpdateStatus() {
	unsigned long timediff;

	timediff = millis() - lastTimeRequest;
	if (timediff > timeSync_interval) {
		timeNeedToBeUpdated = true;
	}

	if (timeRequestInprogress && timediff > timeRequest_timeout) {
		timeNeedToBeUpdated = true;
	}

	if (timeNeedToBeUpdated && !timeRequestInprogress) {
		logger.println(tag, F("\n\n\n\-----------TIME UPDATE TIMEOUT --------\n\n"));
		setEvent(F("Request time"));
		bool res = requestTime();
		if (!res) { // se fallisce rirpova tra un minuto
			logger.println(tag, F("\n request setting failed"));
			lastTimeRequest = millis() - timeSync_interval + 1 * 60 * 1000;
		}
	}
}

void Shield::checkSettingResquestStatus() {

	unsigned long timediff = millis() - lastSettingRequest;
	if (timediff > settingsRequest_interval) {
		settingsNeedToBeUpdated = true;
	}

	if (settingsRequestInprogress && timediff > settingsRequest_timeout) {
		logger.println(tag, F("\n\n\n\-----------SERVER SETTINGS REQUEST TIMEOUT--------\n\n"));
		readSensorFromFile();
		settingsRequestInprogress = false;
		settingsNeedToBeUpdated = true;
	}

	if (!settingFromServerReceived && settingsNeedToBeUpdated && !settingsRequestInprogress) {
		logger.println(tag, F("\n\n\n\-----------SERVER SETTINGS TIMEOUT--------\n\n"));
		setEvent(F("Request setting"));
		bool res = requestSettingsFromServer();
		if (!res) { // se fallisce carica i dati da file
					// e riprova al prossimo timeout
			logger.println(tag, F("\n request setting failed"));
			lastSettingRequest = millis() - settingsRequest_interval + 1 * 60 * 1000;
			readSensorFromFile();
		}
	}
}

bool Shield::requestSettingsFromServer() {
	logger.print(tag, F("\n\t >>requestSettingsFromServer"));
	setEvent(F("request server setting.."));
	settingsNeedToBeUpdated = false;
	lastSettingRequest = millis();
	Command command;
	bool res = command.requestShieldSettings(getMACAddress(), rebootreason);
	if (res)
		settingsRequestInprogress = true;
	else
		settingsRequestInprogress = false;
	logger.print(tag, F("\n\t <<requestSettingsFromServer res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

void Shield::readSensorFromFile() {
	logger.print(tag, F("\n\t >>readSensorFromFile"));

	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/sensors.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/sensors.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());

				if (json.success()) {
					logger.print(tag, F("\n\t parsed json"));
					logger.printJson(json);
					loadSensors(json);
				}
				else {
					logger.print(tag, F("\n\t failed to load sensors file"));
				}
			}
		}
	}
	else {
		logger.print(tag, F("\n\t failed to mount FS"));
	}
	logger.print(tag, F("\n\t <<readSensorFromFile"));
}

bool Shield::writeSensorToFile(JsonObject& json) {
	File configFile = SPIFFS.open("/sensors.json", "w");
	if (!configFile) {
		logger.print(tag, F("\n\t failed to open config file for writing"));
	}
	logger.printJson(json);
	json.printTo(configFile);
	configFile.close();
}

// carica i settings
// può essere chiamat inn qualunque moneto: quanto riceve i setting dal server
// oppure quando all'inizio la scheda legge i setting dalla eprom
bool  Shield::loadSensors(JsonObject& json) {

	logger.print(tag, F("\n\t>>loadSensors"));

	if (json.containsKey("shieldid")) {
		int shieldid = json["shieldid"];
		logger.print(tag, F("\n\t shieldid="));
		logger.print(tag, shieldid);
		setShieldId(shieldid);
	}
	else {
		logger.print(tag, "\n\t error -ID MISSING");
		return false;
	}

	if (json.containsKey("name")) {
		String name = json["name"];
		logger.print(tag, F("\n\t name="));
		logger.print(tag, name);
		setShieldName(name);
	}

	if (json.containsKey("sensors")) {
		String str = json["sensors"];
		size_t size = str.length();
		DynamicJsonBuffer jsonBuffer;
		JsonArray& sensorjsonarray = jsonBuffer.parseArray(str);

		clearAllSensors(); // serve per inizializzare
		//String sensorstr = sensorjsonarray.getFirst();
		for (int i = 0; i < sensorjsonarray.size(); i++) {
#ifdef ESP8266
			ESP.wdtFeed();
#endif // ESP8266
			logger.print(tag, F("\n\n\t SENSOR: "));
			//logger.printJson(tag, sensorstr);
			sensorjsonarray[i].printTo(Serial);

			Sensor* sensor = SensorFactory::createSensor(sensorjsonarray[i]);
			if (sensor != nullptr) {
				logger.print(tag, F("\n\n\t sensor="));
				logger.print(tag, sensor->toString());
				//sensorList.add(sensor);
				sensors.add(sensor);
				logger.print(tag, F("\n\n\t sensor num="));
				logger.print(tag, sensors.size());
			}
		}
	}
	else {
		logger.print(tag, F("\n\t No sensor found!"));
	}
	//logger.printFreeMem(tag, " end loadsensor");
	logger.print(tag, F("\n\t<<loadSensors\n"));
	return true;
}

#ifdef dopo
void Shield::addSensor(Sensor* pSensor) { // non è usata?????????

	logger.print(tag, F("\n\t >>addSensor "));
	logger.print(tag, pSensor->sensorname);
	sensorList.add((Sensor*)pSensor);
	//sensorList.show();
	logger.print(tag, F("\n\t <<addSensor"));
}
#endif

void Shield::writeRebootReason() {

	logger.println(tag, " >>writeRebootReason config");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["rebootreason"] = getRebootReason();

	File configFile = SPIFFS.open("/reason.json", "w");
	if (!configFile) {
		logger.print(tag, "\n\t failed to open rebootreason file for writing");
		return;
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	logger.print(tag, "\n\t reason file written");
	logger.println(tag, "writeRebootReason config");
	//end save
}

void Shield::readRebootReason() {

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
						setRebootReason(str);
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

