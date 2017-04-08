#include "Shield.h"
#include "DS18S20Sensor.h"
#include "DoorSensor.h"
#include "Logger.h"
#include "HeaterActuator.h"
#include "Command.h"
#include "ESP8266Webduino.h"
#include "JSONArray.h"
#include <Adafruit_ST7735.h>

extern void writeEPROM();
//uint8_t oneWirePin = D4;

OneWire* oneWirePtr;
DallasTemperature* pDallasSensors;

Logger Shield::logger;
String Shield::tag = "Shield";
String Shield::lastRestartDate = "";
String Shield::swVersion = "1.02";

uint8_t Shield::heaterPin = D5;
uint8_t Shield::oneWirePin = D4;
bool Shield::heaterEnabled = true;
bool Shield::temperatureSensorsEnabled = true;

int Shield::id = 0; //// inizializzato a zero perchè viene impostato dalla chiamata a registershield
int Shield::serverPort = 8080;
int Shield::localPort = 80;
String Shield::networkSSID;
String Shield::networkPassword;
String Shield::serverName;//[serverNameLen];
String Shield::shieldName;
String Shield::powerStatus = "on";


int Shield::ioDevices[maxIoDevices] = { 0,0,0,0,0,0,0,0,0,0 };

Shield::Shield()
{
	serverPort = 8080;
	//addOneWireSensors();

	//Actuator* pActuator = new Actuator();
	//actuatorList.push_back(*pActuator);
}

Shield::~Shield()
{
}

void Shield::init() {

	tftDisplay.init();
	//display.init();
}

void Shield::clearAllSensors() {

	sensorList.clearAll();
}


void Shield::drawString(int x, int y, String txt, int size, int color) {

	//String str = "prova";
	tftDisplay.drawString(x, y, txt, 1, ST7735_WHITE);
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
	tftDisplay.clear();

	tftDisplay.drawXBitmap(90, 50, temperature_bits, temperature_width, temperature_height, 0xF800/*ST3577_RED*/);
}


String Shield::sendUpdateSensorListCommand(JSON json) {

	logger.print(tag, "\n\t >>sendUpdateSensorListCommand");

	bool writeChanges = false;

	sensorList.clearAll();

	if (json.has("sensors")) {

		String str = json.jsonGetArrayString("sensors");
		//logger.print(tag, "\n\t str=" + str);

		JSONArray jArray(str);
		String item = jArray.getFirst();
		while (!item.equals("")) {
			logger.print(tag, "\n\n\t sensor=" + item);
			
			JSON sensorJson(item);

			if (sensorJson.has("type")) {
				String type = sensorJson.jsonGetString("type");
				logger.print(tag, "\n\t type=" + type);
				if (type.equals("onewiresensor")) {
					addOneWireSensors("1;2;3;4");
				}
				else if (type.equals("doorsensor")) {
					DoorSensor* pDoorSensor = new DoorSensor();
					if (sensorJson.has("name"))
						pDoorSensor->sensorname = sensorJson.jsonGetString("name");
					/*if (sensorJson.has("pin"))
						pDoorSensor->pin = sensorJson.jsonGet("name");*/
					addDoorSensor(pDoorSensor);
				}
			}
			else {
				logger.print(tag, "\n\t type nor fount");
			}

			item = jArray.getNext();
		}		
	}
	
	// disabilitato temporaneamente writeeprom
	writeChanges = false;
	if (writeChanges)
		writeEPROM();

	String result = "";
	result += "{";
	result += "\"result\": \"succes\"";
	result += ",\"temperaturepin\": \"" + getStrHeaterPin() + "\"";
	result += ",\"temperaturesensorsenabled\": ";
	if (getTemperatureSensorsEnabled() == true)
		result += "true";
	else
		result += "false";

	result += "}";

	logger.print(tag, "\n\t <<sendUpdateSensorListCommand");
	return result;
}

String Shield::sendTemperatureSensorsSettingsCommand(JSON json) {

	logger.print(tag, "\n\t >>sendSensorsSettingsCommand");

	bool writeChanges = false;
	if (json.has("temperaturepin")) {
		String str = json.jsonGetString("temperaturepin");
		logger.print(tag, "\n\tpin=" + str);
		if (str.equals("D0"))
			setOneWirePin(D0);
		if (str.equals("D1"))
			setOneWirePin(D1);
		else if (str.equals("D2"))
			setOneWirePin(D2);
		else if (str.equals("D3"))
			setOneWirePin(D3);
		else if (str.equals("D4"))
			setOneWirePin(D4);
		else if (str.equals("D5"))
			setOneWirePin(D5);
		else if (str.equals("D6"))
			setOneWirePin(D6);
		else if (str.equals("D7"))
			setOneWirePin(D7);
		else if (str.equals("D8"))
			setOneWirePin(D8);
		else if (str.equals("D9"))
			setOneWirePin(D9);
		else if (str.equals("D10"))
			setOneWirePin(D10);

		writeChanges = true;
	}
	if (json.has("temperaturesensorsenabled")) {
		bool res = json.jsonGetBool("temperaturesensorsenabled");
		logger.print(tag, "\n\t temperaturesensorsenabled= ");
		if (res)
			logger.print(tag, "true");
		else
			logger.print(tag, "false");
		setTemperatureSensorsEnabled(res);
		writeChanges = true;
	}
	if (json.has("addr") && json.has("name")) {
		String name = json.jsonGetString("name");
		String addr = json.jsonGetString("addr");
		logger.print(tag, "\n\t addr=" + addr + "name=" + name);

		for (int i = 0; i < sensorList.count; i++) {
			if (!sensorList.get(i)->type.equals("temperature"))
				continue;
			DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);
			if (pSensor->getSensorAddress().equals(addr)) {
				pSensor->sensorname = name;
				writeChanges = true;
			}
		}

	}
	if (writeChanges)
		writeEPROM();

	String result = "";
	result += "{";
	result += "\"result\": \"succes\"";
	result += ",\"temperaturepin\": \"" + getStrHeaterPin() + "\"";
	result += ",\"temperaturesensorsenabled\": ";
	if (getTemperatureSensorsEnabled() == true)
		result += "true";
	else
		result += "false";
	result += "}";

	logger.print(tag, "\n\t <<sendSensorsSettingsCommand");
	return result;
}

String Shield::sendHeaterSettingsCommand(JSON json) {

	logger.print(tag, "\n\t>>sendHeaterSettingsCommand");
	if (json.has("heaterpin")) {
		String str = json.jsonGetString("heaterpin");
		logger.print(tag, "\n\tpin=" + str);
		if (str.equals("D0"))
			setHeaterPin(D0);
		if (str.equals("D1"))
			setHeaterPin(D1);
		else if (str.equals("D2"))
			setHeaterPin(D2);
		else if (str.equals("D3"))
			setHeaterPin(D3);
		else if (str.equals("D4"))
			setHeaterPin(D4);
		else if (str.equals("D5"))
			setHeaterPin(D5);
		else if (str.equals("D6"))
			setHeaterPin(D6);
		else if (str.equals("D7"))
			setHeaterPin(D7);
		else if (str.equals("D8"))
			setHeaterPin(D8);
		else if (str.equals("D9"))
			setHeaterPin(D9);
		else if (str.equals("D10"))
			setHeaterPin(D10);
	}
	if (json.has("heaterenabled")) {
		bool res = json.jsonGetBool("heaterenabled");
		logger.print(tag, "\n\t heaterenabled= ");
		if (res)
			logger.print(tag, "true");
		else
			logger.print(tag, "false");
		setHeaterEnabled(res);
	}
	writeEPROM();

	String result = "";
	result += "{";
	result += "\"result\": \"succes\"";
	result += ",\"heaterpin\": \"" + getStrHeaterPin() + "\"";
	result += ",\"heaterenabled\": ";
	if (getHeaterEnabled() == true)
		result += "true";
	else
		result += "false";
	result += "}";
	logger.print(tag, "\n\t<<sendHeaterSettingsCommand");
	return result;
}

String Shield::sendCommand(String jsonStr) {

	logger.print(tag, "\n\t >>sendCommand\njson=" + jsonStr);

	JSON json(jsonStr);
	if (json.has("command")) {

		String command = json.jsonGetString("command");
		if (command.equals("heatersettings")) {
			logger.print(tag, "\n\t ++heatersettings");
			String result = sendHeaterSettingsCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		if (command.equals("temperaturesensorsettings")) {
			logger.print(tag, "\n\t ++temperaturesensorsettings");
			String result = sendTemperatureSensorsSettingsCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (command.equals("updatesensorlist")) {
			logger.print(tag, "\n\t ++updatesensorlist");
			String result = sendUpdateSensorListCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (command.equals("shieldsettings")) {
			logger.print(tag, "\n\t ++shieldsettings");
			String result = sendShieldSettingsCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (command.equals("power")) {
			logger.print(tag, "\n\t ++power");
			String result = sendPowerCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (command.equals("reset")) {
			logger.print(tag, "\n\t ++reset");
			String result = sendResetCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (command.equals("register")) {
			logger.print(tag, "\n\t ++register");
			String result = sendRegisterCommand(json);
			logger.print(tag, "\n\t <<sendCommand result=" + String(result));
			return result;
		}
		else if (heaterEnabled /*&& json.has("actuatorid")*/) { // se arriva direttamente dalla scheda non c'è actuatorid
			/*int actuatorId = json.jsonGetInt("actuatorid");
			logger.print(tag, F("\n\tactuatorid="));
			logger.print(tag, actuatorId);*/
			logger.print(tag, "\n\t ++heater command");

			String result = hearterActuator.sendCommand(jsonStr);
			logger.print(tag, "\n\t<<sendCommand result=" + String(result));
			return result;
		}
	}

	logger.println(tag, "\n\t command not valid");
	String result = "";
	result += "{";
	result += "\"result\": \"failed\"";
	result += "}";
	logger.println(tag, "\n\t<<sendCommand result=" + String(result));
	return result;
}

String Shield::sendShieldSettingsCommand(JSON json)
{
	logger.print(tag, "\n\t>>sendShieldSettingsCommand");

	if (json.has("localport")) {
		int localPortr = json.jsonGetInt("localport");
		logger.print(tag, "\n\tlocalport=" + localPort);
		setLocalPort(localPort);
	}
	if (json.has("shieldname")) {
		String name = json.jsonGetString("shieldname");
		logger.print(tag, "\n\tshieldname=" + name);
		setShieldName(name);
	}
	if (json.has("ssid")) {
		String name = json.jsonGetString("ssid");
		logger.print(tag, "\n\tssid=" + name);
		setNetworkSSID(name);
	}
	if (json.has("password")) {
		String name = json.jsonGetString("password");
		logger.print(tag, "\n\tshieldname=" + name);
		setNetworkPassword(name);
	}
	if (json.has("servername")) {
		String name = json.jsonGetString("servername");
		logger.print(tag, "\n\tshieldname=" + name);
		setServerName(name);
	}
	if (json.has("serverport")) {
		int serverPort = json.jsonGetInt("serverport");
		logger.print(tag, "\n\tserverPort=" + serverPort);
		setServerPort(serverPort);
	}
	writeEPROM();

	String result = "";
	result += "{";
	result += "\"result\": \"succes\"";

	result += ",\"localport\": \"" + String(getLocalPort()) + "\"";
	result += ",\"shieldname\": \"" + getShieldName() + "\"";
	result += ",\"ssid\": \"" + getNetworkSSID() + "\"";
	result += ",\"password\": \"" + getNetworkPassword() + "\"";
	result += ",\"servername\": \"" + getServerName() + "\"";
	result += ",\"serverport\": \"" + String(getServerPort()) + "\"";


	result += "}";

	logger.print(tag, "\n\t<<sendShieldSettingsCommand");
	return result;

}

String Shield::sendRegisterCommand(JSON json)
{
	logger.print(tag, "\n\t>> sendRegisterCommand");

	Command command;
	int id = command.registerShield(*this);

	String result = "";
	result += "{";

	if (id != 0) {
		result += "\"result\": \"succes\"";
		result += ",\"power\": \"" + String(Shield::getShieldId()) + "\"";
	}
	else {
		result += "\"result\": \"failed\"";
	}

	result += "}";

	logger.print(tag, "\n\t<< sendRegisterCommand");
	return result;
}

String Shield::sendResetCommand(JSON json)
{
	logger.print(tag, "\n\t>> sendResetCommand");

	ESP.restart();


	return "";
}

String Shield::sendPowerCommand(JSON json)
{
	logger.print(tag, "\n\t>> sendPowerCommand");
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

	String result = "";
	result += "{";

	if (res) {
		result += "\"result\": \"succes\"";
		result += ",\"power\": \"" + getPowerStatus() + "\"";
	}
	else {
		result += "\"result\": \"failed\"";
	}

	result += "}";

	logger.print(tag, "\n\t<< sendPowerCommand");
	return result;
}

String Shield::getTemperatureSensorsStatusJson() {

	logger.print(tag, "\n\t >>getTemperatureSensorsStatusJson");

	String json = "{";
	json += "\"id\":" + String(id);// shieldid

	json += ",\"temperaturesensorsenabled\":";
	if (Shield::getTemperatureSensorsEnabled() == true)
		json += "true";
	else
		json += "false";
	json += ",\"temperaturesensorspin\":\"" + Shield::getStrOneWirePin() + "\"";


	json += ",\"sensors\":[";

	//if (getTemperatureSensorsEnabled) {
	if (Shield::getTemperatureSensorsEnabled() == true) {
		for (int i = 0; i < sensorList.count; i++) {
			if (!sensorList.get(i)->type.equals("temperature"))
				continue;
			DS18S20Sensor* sensor = (DS18S20Sensor*)sensorList.get(i);
			if (i != 0)
				json += ",";
			json += sensor->getJSON();
		}
	}
	//}

	json += "]";
	json += "}";
	logger.print(tag, "\n\t <<getTemperatureSensorsStatusJson" + json);

	return json;
}

String Shield::getSensorsStatusJson() {

	logger.print(tag, "\n\t >>getSensorsStatusJson");

	String json = "{";
	json += "\"shieldid\":" + String(id);// shieldid

	json += ",\"sensors\":[";	
	// onwire sensore
	String onewire = getOneWireJson();
	if (!onewire.equals(""))
		json += onewire;
	// no onewiresensor
	for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = sensorList.get(i);
		if (sensor->type.equals("temperature"))
			continue;
		if (i != 0 || !onewire.equals(""))
			json += ",";

		json += "{";
		json += "\"type\":\"" + sensor->type + "\"";
		json += ",\"name\":\"" + sensor->sensorname + "\"";
		json += ",\"enabled\":";
		if (sensor->enabled == true)
			json += "true";
		else
			json += "false";
		json += ",\"pin\":\"" + getStrPin(sensor->pin) + "\"";		
		json += "}";
		//json += sensor->getJSON();
	}	
	json += "]";

	//actuators
	json += ",\"actuators\":[";
	if (heaterEnabled) {
		json += hearterActuator.getJSON();
	}
	json += "]";


	json += "}";
	logger.print(tag, "\n\t <<getSensorsStatusJson" + json);

	return json;
}

String Shield::getOneWireJson() {

	logger.print(tag, "\n\t >>String Shield::getOneWireJson()");

		
	String json = "{";
	json += "\"enabled\":" + String(temperatureSensorsEnabled);
	json += ",\"type\":\"onewiresensor\"";
	json += ",\"name\":\"" + String("onewiresensor") + "\"";
	json += ",\"pin\":\"" + getStrOneWirePin() + "\"";
	json += ",\"temperaturesensors\":[";

	bool noOnewire = true;
	for (int i = 0; i < sensorList.count; i++) {
		Sensor* sensor = sensorList.get(i);
		if (!sensor->type.equals("temperature"))
			continue;
		noOnewire = false;
		if (i != 0)
			json += ",";
		//json += sensor->getJSON();
		json += "{";
		json += "\"type\":\"" + sensor->type + "\"";
		json += ",\"name\":\"" + sensor->sensorname + "\"";
		json += "}";


	}
	if (noOnewire)
		return "";
		
	json += "]";
	json += "}";
	logger.print(tag, "\n\t <<getOneWireJson" + json);
	return json;
}

String Shield::getActuatorsStatusJson() {

	logger.print(tag, "\n\t >>getActuatorsStatusJson");
	String json = "{";
	json += "\"id\":" + String(id);// shieldid
	json += ",\"actuators\":[";

	if (heaterEnabled) {
		json += hearterActuator.getJSON();
	}
	json += "]";
	json += "}";
	logger.print(tag, "\n\t <<getActuatorsStatusJson" + json);
	return json;
}

String Shield::getSettingsJson() {
	String json = "{";

	json += "\"localport\":" + String(localPort);
	json += ",\"shieldname\":\"" + shieldName + "\"";
	json += ",\"ssid\":\"" + networkSSID + "\"";
	json += ",\"password\":\"" + networkPassword + "\"";
	json += ",\"servername\":\"" + serverName + "\"";
	json += ",\"serverport\":" + String(serverPort);
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


String Shield::getHeaterStatusJson() {

	logger.print(tag, "\n\t >>getHeaterStatusJson");
	String json = hearterActuator.getJSON();
	logger.print(tag, "\n\t <<getHeaterStatusJson" + json);
	return json;
}

void Shield::checkStatus()
{


	checkActuatorsStatus();
	checkSensorsStatus();

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
	int textWidth = 5;
	int textHeight = 8;

	tftDisplay.drawString(0, 0, Logger::getStrDayDate() + " ", 1, ST7735_WHITE);
	tftDisplay.drawString(0, textHeight, Logger::getStrTimeDate() + " ", 2, ST7735_WHITE);

	tftDisplay.drawString(0, textHeight*(2 + 1), hearterActuator.getStatusName(), 2, ST7735_WHITE);

	String releStatus = "spento";
	if (hearterActuator.getReleStatus() == 1)
		releStatus = "acceso";
	tftDisplay.drawString(0, textHeight*(4 + 1), releStatus, 2, ST7735_WHITE);

	for (int i = 0; i < sensorList.count; i++) {
		if (!sensorList.get(i)->type.equals("temperature"))
			continue;
		DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);
		tftDisplay.drawString(0, textHeight * (2 + 1 + 2 + 2) + textHeight * i * 1, pSensor->sensorname + " " + pSensor->getTemperature(), 0, ST7735_WHITE);
	}
	//display.update();

}

void Shield::checkActuatorsStatus()
{
	if (heaterEnabled) {
		hearterActuator.checkStatus();
	}
}

void Shield::checkSensorsStatus()
{
	//logger.println(tag, F(">>checkSensorsStatus"));
	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckTemperature;
	if (timeDiff > checkTemperature_interval) {

		lastCheckTemperature = currMillis;
		checkTemperatures();
		return;
	}
}

void Shield::addOneWireSensors(String sensorNames) {

	logger.println(tag, ">>addOneWireSensors");

	logger.print(tag, "\n\t OneWirepin=" + String(oneWirePin));

	oneWirePtr = new OneWire(oneWirePin);
	pDallasSensors = new DallasTemperature(oneWirePtr);

	pDallasSensors->begin();
	
	//sensorList.init();

	uint8_t address[8];
	int count = 0;
	while (oneWirePtr->search(address)) {
		logger.print(tag, "\n\tFound \'1-Wire\' device with address:");
		for (int i = 0; i < 8; i++) {
			logger.print(tag, "0x");
			if (address[i] < 16) {
				logger.print(tag, '0');
			}
			logger.print(tag, address[i]);
			if (i < 7) {
				logger.print(tag, ", ");
			}
		}
		if (OneWire::crc8(address, 7) != address[7]) {
			logger.print(tag, "\n\tCRC is not valid!");
			return;
		}

		logger.print(tag, "\n\t sensorNames=" + sensorNames);

		int index = sensorNames.indexOf(";");
		int n = 0;
		int start = 0;
		String name = "temp sensor";

		while (index != -1 && n <= count) {
			name = sensorNames.substring(start, index);
			start = index+1;
			index = sensorNames.indexOf(";",start);
			n++;
		}

		/*String name = "";
		int index = sensorNames.indexOf(";");
		if (index >= 0) {
			name = sensorNames.substring(0, index);
			if (index < sensorNames.length() - 1)
				sensorNames = sensorNames.substring(index + 1);
			else
				sensorNames = "";
		}
		else {
			name = "sensor" + String(count);
		}*/

		DS18S20Sensor* pSensorNew = new DS18S20Sensor();
		pSensorNew->sensorname = name/* + String(count)*/;

		for (int i = 0; i < 8; i++) {
			pSensorNew->sensorAddr[i] = address[i];
		}

		//logger.print(tag, "\n\tpSensorNew->sensorAddr=");
		//logger.print(tag, pSensorNew->getSensorAddress());

		count++;
		sensorList.add((Sensor*)pSensorNew);
		//sensorList.show();

		//String txt = "\n\tADDDED sensor " + String(pSensorNew->sensorname) + "addr ";
		//logger.print(tag, txt);
		//logger.print(tag, pSensorNew->getSensorAddress());
		//logger.print(tag, " end");
	}
	oneWirePtr->reset_search();
	sensorList.show();
	logger.println(tag, "<<addOneWireSensors");
}

void Shield::addDoorSensor(DoorSensor* pDoorSensor) {

	logger.println(tag, ">>addDoorSensor");
		
	/*DoorSensor* pDoorSensor = new DoorSensor();
	pDoorSensor->sensorname = "DoorSensor";*/
	
	sensorList.add((Sensor*)pDoorSensor);
	sensorList.show();
	
	logger.println(tag, "<<addDoorSensor");
}

void Shield::addActuators() {

	logger.println(tag, "addActuator...\n\r");
	ActuatorList.init();

	if (heaterEnabled) {
		//hearterActuator.setRelePin(heaterPin);
	}
	//HeaterActuator* pActuatorNew = new HeaterActuator();
	//pActuatorNew->sensorname = "riscaldamento";
}


void Shield::checkTemperatures() {

	logger.print(tag, "\n");
	logger.println(tag, ">>checkTemperatures---");

	//sensorList.show();
	float oldTemperature;
	for (int i = 0; i < sensorList.count; i++) {
		if (!sensorList.get(i)->type.equals("temperature"))
			continue;

		DS18S20Sensor* pSensor = (DS18S20Sensor*)sensorList.get(i);
		oldTemperature = pSensor->getTemperature();

		//sensorList.show();
		pSensor->readTemperature(pDallasSensors);

		if (oldTemperature != pSensor->getTemperature()) {
			temperatureChanged = true;
			logger.print(tag, "\ttemperatura cambiata");
			logger.print(tag, String(oldTemperature));
			logger.print(tag, "->");
			logger.print(tag, String(pSensor->getTemperature()));
		}

		// imposta la temperatura locale a quella del primo sensore (DA CAMBIARE)
		if (i == 0)
			hearterActuator.setLocalTemperature(pSensor->getTemperature());
		//sensorList.show();
	}

	// se il sensore attivo è quello locale aggiorna lo stato
	// del rele in base alla temperatur del sensore locale
	//if (!hearterActuator.sensorIsRemote())
	hearterActuator.updateReleStatus();

	Command command;
	if (temperatureChanged) {

		logger.print(tag, "\n\n\tSEND TEMPERATURE UPDATE - temperature changed\n");
		//logger.print(tag, "\n\toldLocalTemperature=");

		logger.println(tag, ">>command.sendSensorsStatus");
		command.sendSensorsStatus(*this);
		logger.println(tag, "<<command.sendSensorsStatus");
		temperatureChanged = false; // qui bisognerebbe introdiurre il controllo del valore di ritorno della send
									// cokmmand ed entualmente reinviare
	}

	logger.println(tag, "<<checkTemperatures\n");
	logger.print(tag, "\n");
}

