#include "Shield.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "Logger.h"
#include "Command.h"
#include "ESP8266Webduino.h"
//#include "JSONArray.h"
#ifdef ESP8266
#include <Adafruit_ST7735.h>
#endif
#include "SensorFactory.h"

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//extern void writeEPROM();
//extern void resetEPROM();
extern void resetWiFiManagerSettings();
extern bool mqtt_publish(String topic, String message);

Logger Shield::logger;
String Shield::tag = "Shield";

Shield::Shield()
{
	lastRestartDate = "";
	swVersion = "1.66";

	id = 0; //// inizializzato a zero perch� viene impostato dalla chiamata a registershield
	localPort = 80;
	serverName = "giacomohome.ddns.net";
	mqttServer = "giacomohome.ddns.net";
	mqttPort = 1883;
	serverPort = 8080;
	shieldName = "shieldName";
	//MAC_array[6];
	//MAC_char[18];
	mqttMode = true;// true;
	configMode = false;// true;
	resetSettings = false;// true;
	powerStatus = "on"; // da aggiungere

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
	//logger.print(tag, "\n\t >>getSensorFromId " + String(id));
	//logger.print(tag, "\n\t sensorList.count= " + String(sensorList.count));
	for (int i = 0; i < sensorList.count; i++) {
		//logger.print(tag, "\n\t i= " + String(i));
		Sensor* sensor = (Sensor*)sensorList.get(i);
		//logger.print(tag, "\n\t sensor->sensorid= " + String(sensor->sensorid));
		//logger.print(tag, " sensor->sensorname= " + sensor->sensorname);
		if (sensor->sensorid == id) {
			//logger.print(tag, "\n\t sensor found " + sensor->sensorname);
			//logger.print(tag, "\n\t <<getSensorFromId");
			return sensor;
		}
		//logger.print(tag, "\n\t childsensors.length()= " + String(sensor->childsensors.length()));
		for (int k = 0; k < sensor->childsensors.length(); k++) {
			//logger.print(tag, "\n\t child k= " + String(k));
			Sensor* child = (Sensor*)sensor->childsensors.get(k);
			//logger.print(tag, "\n\t childsensors= " + String(child->sensorid));
			//logger.print(tag, " name= " + String(child->sensorname));
			if (child->sensorid == id) {
				//logger.print(tag, "\n\t sensor child found " + child->sensorname);
				//logger.print(tag, "\n\t <<getSensorFromId");
				return (Sensor*)child;
			}
		}
	}
	//logger.println(tag, "\n\t sensor not found");
	//logger.print(tag, "\n\t <<getSensorFromId");
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
	for (int i = 0; i < sensorList.count; i++) {

		Sensor* sensor = (Sensor*)sensorList.get(i);
		if (!sensor->enabled)
			continue;
		String txt = sensor->sensorname.substring(0, 5);
		espDisplay.drawString(0, lines++ * 10, txt + ": " + String(sensor->getStatusText()));
		for (int k = 0; k < sensor->childsensors.length(); k++) {
			
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

#ifdef DOPO
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

#endif // DOPO
void Shield::parseMessageReceived(String topic, String message) {

	//logger.print(tag, "\n\t >>parseMessageReceived");
	String str = String(F("fromServer/shield/")) + getMACAddress();

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
			json.printTo(Serial);
		}
		else {
			logger.print(tag, F("failed to load json config"));
			return;
		}
		
		if (loadSensors(json)) {
			writeSensorToFile(json);
			//writeEPROM();
			settingFromServerReceived = true;
			Command command;
			command.sendShieldStatus(getJson());
		}
	}
	else if (topic.equals(str + F("/reboot"))) {
		logger.print(tag, F("\n\t received reboot request"));
		setEvent(F("<-received reboot request"));
		ESP.restart();
	}
	else if (topic.equals(str + F("/time"))) { // risposta a time
		logger.print(tag, F("\n\t received time"));
		setEvent(F("<-received time"));
		lastTimeSync = millis();
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
	else if (str.equals(F("reset"))) {
		logger.print(tag, F("\n\t RESET SETTINGS!!!!!!!!!!!!!!!!"));
		setResetSettings(true);
		//resetEPROM();
		ESP.restart();
	}
	else {
		logger.print(tag, F("\n\t PARSE NOT FOUND"));
	}
	logger.printFreeMem(tag, F("parseMessageReceived"));
	logger.print(tag, F("\n\t <<parseMessageReceived"));
}

#ifdef dopo
bool Shield::receiveCommand(String jsonStr) {

	logger.print(tag, F("\n\t >>receiveCommand"));
	//logger.printFreeMem(tag, "++receiveCommand");
	logger.print(tag, jsonStr);

	JSON json(jsonStr);
	if (json.has(F("command"))) {
		bool result = false;
		String command = json.jsonGetString("command");
		logger.print(tag, F("\n\t command="));
		logger.print(tag, command);
		setEvent("<-received command " + command);
		/*if (command.equals("updatesensorlist")) { // ???
		result = onUpdateSensorListCommand(json);
		logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
		return result;
		}
		else */if (command.equals(F("shieldsettings"))) { // risposta a loadsettting
			result = onShieldSettingsCommand(json);
			logger.print(tag, "\n\t <<receiveCommand result=" + String(result));
			return result;
		}
		else if (command.equals(F("power"))) { // ??
			result = onPowerCommand(json);
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return result;
		}
		else if (command.equals(F("reset"))) { // che differenza c'è tra reboot e reset?
			result = onResetCommand();
			//result = onRebootCommand(json);
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return result;
		}
		else if (command.equals(F("reboot"))) { // che differenza c'è tra reboot e reset?
			result = onRebootCommand();
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return result;
		}
		else if (command.equals(F("checkhealth"))) {
			logger.printFreeMem(tag, F("++checkhealth"));

			// uuid
			String uuid = "";
			if (json.has(F("uuid"))) {
				uuid = json.jsonGetString(F("uuid"));
				logger.print(tag, F("\n\t uuid="));
				logger.print(tag, uuid);
			}
			else {
				logger.print(tag, F("\n\t uuid NOT FOUND!"));
				return false;
			}

			lastCheckHealth = millis();
			String topic = "toServer/response/" + uuid + "/success";
			bool res = mqtt_publish(topic, getJson());

			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return res;
		}
		else if (json.has(F("actuatorid")) && json.has(F("uuid"))) {	// se c'è il campo actuatorid

			logger.print(tag, F("\n\treceived actuator command"));
			//logger.printFreeMem(tag, "++actuatorcommand");				// allora è un comando per un sensore			
			int id = json.jsonGetInt("actuatorid");
			logger.print(tag, F("\n\t actuatorid="));
			logger.print(tag, String(id));
			String uuid = json.jsonGetString(F("uuid"));
			Sensor* sensor = getSensorFromId(id);
			if (sensor != nullptr) {
				//logger.printFreeMem(tag, "++receiveCommand");
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
	logger.print(tag, F("\n\t <<receiveCommand"));
	return false;
}
#endif

bool Shield::receiveCommand(String jsonStr) {

	logger.print(tag, F("\n\t >>receiveCommand"));
	//logger.printFreeMem(tag, "++receiveCommand");
	logger.print(tag, jsonStr);

	size_t size = jsonStr.length();
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(jsonStr);
	if (json.success()) {
		logger.print(tag, F("\n\t settings= \n"));
		json.printTo(Serial);
	}
	else {
		Serial.println("failed to load json config");
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
		else if (command.equals(F("power"))) { // ??
			result = onPowerCommand(json);
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return result;
		}
		else if (command.equals(F("reset"))) { // che differenza c'è tra reboot e reset?
			result = onResetCommand();
			//result = onRebootCommand(json);
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return result;
		}
		else if (command.equals(F("reboot"))) { // che differenza c'è tra reboot e reset?
			result = onRebootCommand();
			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
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
			String topic = "toServer/response/" + uuid + "/success";
			bool res = mqtt_publish(topic, getJson());

			logger.print(tag, F("\n\t <<receiveCommand result="));
			logger.print(tag, String(result));
			return res;
		}
		else if (json.containsKey("actuatorid") && json.containsKey("uuid")) {	// se c'è il campo actuatorid

			logger.print(tag, F("\n\treceived actuator command"));
			int id = json["actuatorid"];
			logger.print(tag, F("\n\t actuatorid="));
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
	logger.print(tag, F("\n\t <<receiveCommand"));
	return false;
}

#ifdef dopo
bool Shield::onShieldSettingsCommand(JSON& json)
{
	logger.print(tag, F("\n\t>>onShieldSettingsCommand"));

	if (json.has(F("localport"))) {
		int localPortr = json.jsonGetInt("localport");
		//logger.print(tag, "\n\t localport=" + localPort);
		setLocalPort(localPort);
	}
	if (json.has(F("shieldname"))) {
		String name = json.jsonGetString("shieldname");
		//logger.print(tag, "\n\t shieldname=" + name);
		setShieldName(name);
	}
	if (json.has(F("servername"))) {
		String name = json.jsonGetString("servername");
		//logger.print(tag, "\n\t servername=" + name);
		setServerName(name);
	}
	if (json.has(F("serverport"))) {
		int serverPort = json.jsonGetInt("serverport");
		//logger.print(tag, "\n\t serverport=" + serverPort);
		setServerPort(serverPort);
	}
	writeEPROM();

	logger.print(tag, F("\n\t <<onShieldSettingsCommand"));
	return true;
}
#endif

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
	//writeEPROM();

	logger.print(tag, F("\n\t <<onShieldSettingsCommand"));
	return true;
}


bool Shield::onRebootCommand()
{
	logger.print(tag, F("\n\t>> onRebootCommand"));
	ESP.restart();
	return true;
}

bool Shield::onResetCommand()
{
	logger.print(tag, F("\n\t>> onResetCommand"));
	resetWiFiManagerSettings();
	//resetEPROM(); // scrive 0 nel primo settore della EPROM per azzerare tutto

	return true;
}

#ifdef dopo
bool Shield::onPowerCommand(JSON& json)
{
	logger.print(tag, F("\n\t>> onPowerCommand"));
	bool res = false;

	if (json.has(F("status"))) {
		String status = json.jsonGetString(F("status"));
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
#endif 

bool Shield::onPowerCommand(JsonObject& json)
{
	logger.print(tag, F("\n\t>> onPowerCommand"));
	bool res = false;

	if (json.containsKey("status")) {
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

String Shield::getJson() {

	logger.print(tag, F("\n\t >>Shield::getJson"));

	// andrebbe trasformato in JSONObject
	String str = "{";

	str += "\"MAC\":\"" + String(MAC_char) + "\"";
	str += ",\"swversion\":\"" + swVersion + "\"";
	str += ",\"lastreboot\":\"" + lastRestartDate + "\"";
	str += ",\"lastcheckhealth\":\"" + String(lastCheckHealth) + "\"";
	str += ",\"freemem\":\"" + String(freeMemory) + "\"";
	str += ",\"localIP\":\"" + localIP + "\"";
	str += ",\"localPort\":\"" + String(Shield::getLocalPort()) + "\"";
	str += "}";
	
	logger.print(tag, F("\n\t <<Shield::getJson json="));
	logger.print(tag, str);
	return str;
}

#ifdef dopo
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
		//logger.println(tag, "\n\n\t json length=" + String(json.length()));
	}
	json += "]";
	json += "}";

	logger.println(tag, F("<<getSensorsStatusJson"));
	return json;
}
#endif

#ifdef dopo
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
#endif

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
			setEvent(F("SENSOR STATUS CHANGED"));

			logger.println(tag, F(">>SEND SENSOR STATUS"));
			logger.print(tag, F("\n\t sensor->name: "));
			logger.print(tag, String(sensor->sensorname));
			if (res) {
				logger.print(tag, F("\n\t status changed"));
				logger.print(tag, String(sensor->status));
			}
			if (timeDiff > sensor->updateStatus_interval)
				logger.print(tag, F("\n\t updateStatus_interval timeout"));

			//sensorStatusChanged = true;
			sensor->lastUpdateStatus = millis();

			Command command;
			bool res = command.sendSensorStatus(sensor->getJSON());
			sensor->lastUpdateStatusFailed = !res;
			logger.println(tag, F("<<SEND SENSOR STATUS"));
		}
	}
}

#ifdef dopo
// carica i settings
// può essere chiamat inn qualunque moneto: quanto riceve i setting dal server
// oppure quando all'inizio la scheda legge i setting dalla eprom
bool  Shield::loadSensors(String settings) {

	//logger.print(tag, "\n\n");
	logger.print(tag, F("\n\t>>loadSensors"));
	//logger.printFreeMem(tag, "loadSensors");
	logger.print(tag, F("\n\t settings="));
	logger.print(tag, settings);

	JSONObject json(settings);

	if (json.has("shieldid")) {
		int shieldid = json.getInteger("shieldid");
		setShieldId(shieldid);

	}
	else {
		logger.println(tag, "error -ID MISSING");
		return false;
	}

	if (json.has("name")) {
		String name = json.getString("name");
		setShieldName(name);
	}

	//logger.printFreeMem(tag, "start load sensor");
	if (json.has("sensors")) {
		String str = json.getJSONArray("sensors");
		JSONArray sensors(str);
		//logger.printFreeMem(tag, "");
		clearAllSensors(); // serve per inizializzare
		String sensorstr = sensors.getFirst();
		while (sensorstr != "") {
			//logger.printFreeMem(tag, "++sensor");
#ifdef ESP8266
			ESP.wdtFeed();
#endif // ESP8266
			logger.print(tag, F("\n\n\t SENSOR: "));
			logger.print(tag, sensorstr);
			JSONObject jObject(sensorstr);
			//logger.print(tag, "\n: " + logger.formattedJson(jObject.toString()));
			Sensor* sensor = SensorFactory::createSensor(&jObject);
			if (sensor != nullptr) {
				logger.print(tag, F("\n\n\t sensor="));
				logger.print(tag, sensor->toString());
				sensorList.add(sensor);
			}

			sensorstr = sensors.getNext();
			//logger.printFreeMem(tag, "--sensor");
		}

	}
	//logger.printFreeMem(tag, " end loadsensor");
	logger.println(tag, F("<<loadSensors\n"));
	return true;

}
#endif

bool Shield::updateTime() {
	logger.println(tag, F(">>requestTimeFromServer"));
	Command command;
	bool res = command.requestTime(getMACAddress());
	logger.print(tag, F("\n\t <<requestTimeFromServer res="));
	logger.print(tag, Logger::boolToString(res));
	return res;
}

void Shield::readSensorFromFile() {
	if (SPIFFS.begin()) {
		Serial.println("mounted file system");
		if (SPIFFS.exists("/sensors.json")) {
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
					loadSensors(json);
					//return json;
				}
				else {
					Serial.println("failed to load json config");
					//return json;

				}
			}
		}
	}
	else {
		Serial.println("failed to mount FS");
	}
	//end read
}

bool Shield::writeSensorToFile(JsonObject& json) {
	File configFile = SPIFFS.open("/sensors.json", "w");
	if (!configFile) {
		Serial.println("failed to open config file for writing");
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
}

// carica i settings
// può essere chiamat inn qualunque moneto: quanto riceve i setting dal server
// oppure quando all'inizio la scheda legge i setting dalla eprom
bool  Shield::loadSensors(/*String settings*/JsonObject& json) {

	//logger.print(tag, "\n\n");
	logger.print(tag, F("\n\t>>loadSensors"));
	//logger.printFreeMem(tag, "loadSensors");
	//logger.print(tag, F("\n\t settings="));
	//logger.print(tag, settings);

	/*size_t size = settings.length();
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(settings);
	if (json.success()) {
		logger.print(tag, F("\n\t settings= \n"));
		json.printTo(Serial);
	}
	else {
		Serial.println("failed to load json config");
		return false;
	}*/
	
	if (json.containsKey("shieldid")) {
		int shieldid = json["shieldid"];
		setShieldId(shieldid);
	}
	else {
		logger.println(tag, "error -ID MISSING");
		return false;
	}

	if (json.containsKey("name")) {
		String name = json["name"];
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
			//logger.print(tag, sensorstr);
			sensorjsonarray[i].printTo(Serial);
			
			Sensor* sensor = SensorFactory::createSensor(sensorjsonarray[i]);
			if (sensor != nullptr) {
				logger.print(tag, F("\n\n\t sensor="));
				logger.print(tag, sensor->toString());
				sensorList.add(sensor);
			}

		}

	}
	//logger.printFreeMem(tag, " end loadsensor");
	logger.println(tag, F("<<loadSensors\n"));
	return true;

}

#ifdef dopo
void Shield::addSensor(Sensor* pSensor) { // non è usata?????????

	logger.print(tag, F("\n\t >>addSensor ") );
	logger.print(tag, pSensor->sensorname);
	sensorList.add((Sensor*)pSensor);
	//sensorList.show();
	logger.print(tag, F("\n\t <<addSensor"));
}
#endif
