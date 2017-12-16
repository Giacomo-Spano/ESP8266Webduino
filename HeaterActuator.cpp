#include "HeaterActuator.h"
#include "Logger.h"
#include "Command.h"

Logger HeaterActuator::logger;
String HeaterActuator::tag = "HeaterActuator";

char* HeaterActuator::statusStr[] = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };

//HeaterActuator::HeaterActuator(uint8_t pin, bool enabled, String address, String name) /*: Actuator(pin, enabled, address, name)*/
HeaterActuator::HeaterActuator(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	checkStatus_interval = 10000;
	lastCheckStatus = 0;

	type = "heatersensor";

	lastTemperatureUpdate = "";
}

HeaterActuator::~HeaterActuator()
{
}

CommandResponse HeaterActuator::receiveCommand(String jsonStr)
{
	JSON json(jsonStr);

	// actuatorId
	int actuatorId;
	if (json.has("actuatorid")) {
		actuatorId = json.jsonGetInt("actuatorid");
		logger.print(tag, "\n\t actuatorid=" + actuatorId);
	}
	// command
	String command = "";
	if (json.has("command")) {
		command = json.jsonGetString("command");
		logger.print(tag,"\n\t command=" + command);
	}
	// duration
	int duration = 0;
	if (json.has("duration")) {
		duration = json.jsonGetInt("duration");
		logger.print(tag, "\n\t duration=" + String(duration) + " minuti");
		duration = duration * 60 * 1000;
		logger.print(tag, "\n\t duration=" + String(duration) + " millisecondi");
	}
	// sensor
	int sensorId = 0;
	if (json.has("sensor")) {
		sensorId = json.jsonGetInt("sensor");
		logger.print(tag, "\n\t sensorId=" + String(sensorId));
	}
	// localsensor
	bool localSensor = false;
	if (json.has("localsensor")) {
		localSensor = json.jsonGetBool("localsensor");
		logger.print(tag, "\n\t localsensor=" + localSensor);
	}
	// target
	float target = 0;
	if (json.has("target")) {
		target = json.jsonGetFloat("target");
		logger.print(tag, "\n\t target=" + String(target));
	}
	// remote temperature
	float remoteTemperature = 0;
	if (json.has("temperature")) {
		remoteTemperature = json.jsonGetFloat("temperature");
		logger.print(tag, "\n\t temperature=" + String(remoteTemperature));
	}
	// lastTemperatureUpdate
	String lastTempUpdate = json.jsonGetString("lasttemperatureupdate");
	if (json.has("date")) {
		lastTemperatureUpdate = json.jsonGetString("date");
		logger.print(tag, "\n\t lasttemperatureupdate=" + lastTempUpdate);
	}
	// sensor
	int program = 0;
	if (json.has("program")) {
		program = json.jsonGetInt("program");
		logger.print(tag, "\n\tprogram=" + String(program));
	}
	// sensor
	int timerange = 0;
	if (json.has("timerange")) {
		sensorId = json.jsonGetInt("timerange");
		logger.print(tag, "\n\ttimerange=" + String(timerange));
	}

	logger.print(tag, "\n\t changeProgram param=" + String(remoteTemperature));
	changeProgram(command, duration,
		!localSensor,
		remoteTemperature,
		lastTempUpdate,
		sensorId,
		target, program, timerange);	
	// result
	String jsonResult = "";
	jsonResult += "{";
	jsonResult += "\"result\": \"success\"";
	jsonResult += "}";

	CommandResponse res;
	return res;
}

String HeaterActuator::getJSONFields()
{
	logger.print(tag, "\n\t >>HeaterActuator::getJSONFields");

	String json = "";
	json += Sensor::getJSONFields();

	json += ",\"remotetemperature\":" + String(getRemoteTemperature());
	json += ",\"lasttemperatureupdate\":\"" + lastTemperatureUpdate + "\"";
	json += ",\"status\":\"" + String(statusStr[getStatus()]) + "\"";
	json += ",\"relestatus\":" + String((getReleStatus()) ? "true" : "false");

	if (getStatus() == Program::STATUS_PROGRAMACTIVE
		|| getStatus() == Program::STATUS_MANUAL_AUTO
		|| getStatus() == Program::STATUS_MANUAL_OFF) {

		json += ",\"duration\":";
		json += String(programDuration / 1000);

		int remainingTime = programDuration - (millis() - programStartTime);
		json += ",\"remaining\":";
		json += String(remainingTime / 1000);

		json += ",\"localsensor\":";
		if (!sensorIsRemote())
			json += "true";
		else
			json += "false";

		if (getStatus() == Program::STATUS_MANUAL_AUTO || getStatus() == Program::STATUS_PROGRAMACTIVE) {
			json += ",\"target\":";
			json += String(getTargetTemperature());
		}

		if (getStatus() == Program::STATUS_PROGRAMACTIVE) {
			json += ",\"program\":";
			json += String(getActiveProgram());

			json += ",\"timerange\":";
			json += String(getActiveTimeRange());
		}
	}
	logger.print(tag, "\n\t <<HeaterActuator::getJSONFields json=" + json);
	return json;
}

bool HeaterActuator::getJSON(JSONObject * jObject)
{
	logger.print(tag, "\n");
	logger.println(tag, ">>getJSON");

	bool res = Sensor::getJSON(jObject);
	if (!res) return false;

	/*jObject.pushInteger("id", id);
	jObject.pushString("phisicaladdr", getPhisicalAddress());
	jObject.pushFloat("temperature", temperature);
	jObject.pushFloat("avtemperature", avTemperature);*/

	logger.println(tag, "<<getJSON");
	return res;
}


void HeaterActuator::init()
{
	logger.print(tag, "\n\t >>init HeaterActuator");
	ConsumptionStartTime = 0;// millis();
	pinMode(pin, OUTPUT);
	setStatus(Program::STATUS_IDLE);
	//subaddress += MACAddress;
	
	sensorname = "Riscaldamento";
	logger.print(tag, "\n\t <<init HeaterActuator");
}

bool HeaterActuator::checkStatusChange()
{
	//logger.print(tag, "\n\n\t >>HeaterActuator::checkStatusChange");

	bool sendStatus = false;

	// controlla se il programma attivo è finito.
	if (programEnded())
		sendStatus = true;
	
	// notifica il server se è cambiato lo stato del rele
	if (releStatusChanged()) {

		logger.println(tag, "\t!->rele status changed");
		//HeaterActuator ha = this->;
		//command.sendActuatorStatus(*this);
		saveOldReleStatus();
		sendStatus = true;
	}

	// notifica il server se è cambiato lo status
	if (statusChanged()) {

		logger.println(tag, "\t!->status changed");
		saveOldStatus();
		sendStatus = true;
	}
		
	/*if (sendStatus) {
		Command command;
		command.sendSensorsStatus(get);
	}*/

	//logger.print(tag, "\n\n\t <<HeaterActuator::checkStatusChange sendstatus=" + String(sendStatus));

	return sendStatus;
}

void HeaterActuator::setTargetTemperature(float target)
{
	logger.print(tag, "\n\t >>setTargetTemperature " + String(target));
	targetTemperature = target;
}

float HeaterActuator::getTargetTemperature() {
	return targetTemperature;
}

void HeaterActuator::setRemote(float temp,String lastTempUpdate)
{
	logger.print(tag, "\n\t remoteTemperature" + String(remoteTemperature));
	remoteTemperature = temp;
	last_RemoteSensor = millis();
	lastTemperatureUpdate = lastTempUpdate;
	logger.print(tag, ",last_RemoteSensor=" + String(last_RemoteSensor));
}

float HeaterActuator::getRemoteTemperature() {
	return remoteTemperature;
}

float HeaterActuator::getLocalTemperature()
{
	return localAvTemperature;
}

void HeaterActuator::setSensorRemote(bool enable, int sensorId)
{
	/*logger.print(tag, "\n\t >>setSensorRemote");
	logger.print(tag, "\n\t tenable=" + String(enable));
	logger.print(tag, "\n\t sensorId=" + sensorId);*/

	sensorRemote = enable;
	remoteSensorId = sensorId;

	last_RemoteSensor = millis();
	//logger.print(tag, "\n\t last_RemoteSensor=" + String(last_RemoteSensor));
}

int HeaterActuator::getRemoteSensorId() {
	return remoteSensorId;
}

bool HeaterActuator::sensorIsRemote() {
	return sensorRemote;
}

int HeaterActuator::getLocalSensorId()
{
	return localSensorId;
}

void HeaterActuator::setLocalTemperature(float temperature) {
	localAvTemperature = temperature;
}

bool HeaterActuator::programEnded()
{
	unsigned long currMillis = millis();

	if (currentStatus == Program::STATUS_PROGRAMACTIVE || currentStatus == Program::STATUS_MANUAL_AUTO || currentStatus == Program::STATUS_MANUAL_OFF) {

		/*if (sensorRemote && (currMillis - last_RemoteSensor) > remoteSensorTimeout) {
			// ferma il programma se è passato troppo tempo dall'ultimo aggiornamento ricevuto dal server
			logger.println(tag, "REMOTE SENSOR TIMEOUT");
			// è inutile mandare un sendstatus perchè tanto cambia lo stato dopo e verrebbe inviato due volte
			remoteTemperature = 0;
			setStatus(Program::STATUS_IDLE);			
			return true;
		}
		else */if (currMillis - programStartTime > programDuration) {
			logger.println(tag, "END PROGRAM");
			// � iniutile mandare un sendstatus perch� tanto cambia lo stato dopo e verrebbe inviato due volte
			setStatus(Program::STATUS_IDLE);
			//enableRele(false);
			return true;
		}
	}
	return false;
}

void HeaterActuator::updateReleStatus() {

	logger.print(tag, "\n\n\t>>updateReleStatus");

	logger.print(tag, "\n\t ConsumptionStartTime=" + String(ConsumptionStartTime));
	logger.print(tag, "\n\t totalConsumptionTime=" + String(totalConsumptionTime));
	logger.print(tag, "\n\t lastConsumptionEnableTime=" + String(lastConsumptionEnableTime));
	logger.print(tag, "\n\t targetTemperature=" + String(targetTemperature));
	logger.print(tag, "\n\t sensorRemote=" + String(sensorRemote));
	logger.print(tag, "\n\t remoteSensorId=" + String(remoteSensorId));
	logger.print(tag, "\n\t remoteTemperature=" + String(remoteTemperature));
	logger.print(tag, "\n\n");

	if (currentStatus == Program::STATUS_MANUAL_OFF) {

		logger.print(tag, "\n\t STATUS_MANUAL_OFF");
		enableRele(false);
	}
	else if (currentStatus == Program::STATUS_MANUAL_AUTO) {

		logger.print(tag, "\n\t STATUS_MANUAL_AUTO");
		// se stato manuale accendi il rel� se sensore == locale e temperatura sensore locale < temperatura target oppure
		// se sensore == remoto e temperature sensore remoto < temperatura target 
		if ((!sensorRemote && localAvTemperature < targetTemperature) ||
			(sensorRemote && remoteTemperature < targetTemperature)) {

			logger.print(tag, F("-LOW TEMPERATURE"));
			enableRele(true);
		}
		else {
			logger.print(tag, F("-HIGH TEMPERATURE"));
			enableRele(false);
		}
	}
	else if (currentStatus == Program::STATUS_PROGRAMACTIVE) {

		logger.print(tag, "\n\t STATUS_PROGRAMACTIVE");

		if (sensorRemote) {
			logger.print(tag, F("-REMOTE SENSOR")); // non modificare stato rele, controllato remotamente
		}
		else if (!sensorRemote) {
			logger.print(tag, F("-LOCAL SENSOR"));
			if (localAvTemperature < targetTemperature) {

				logger.print(tag, F("-LOW TEMPERATURE"));
				enableRele(true);
			}
			else {
				logger.print(tag, F("-HIGH TEMPERATURE"));
				enableRele(false);
			}
		}
	}
	else {
		logger.print(tag, "\n\t INACTIVE-rele OFF");
		enableRele(false);
	}

	logger.print(tag, "\n\t <<updateReleStatus\n");
}

void HeaterActuator::changeProgram(String command, long duration, bool sensorRemote, float remotetemperature, String lastTempUpdate, int sensorId, float target, int program, int timerange) {

	logger.print(tag, F("\n\t >>HeaterActuator::changeProgram"));

	/*logger.print(tag, "\n\t currentStatus=" + String(currentStatus));
	logger.print(tag, String("\n\t command=") + command);
	logger.print(tag, String("\n\t duration=") + String(duration));
	logger.print(tag, String("\n\t sensorRemote=") + String(sensorRemote));
	logger.print(tag, String("\n\t remotetemperature=") + String(remotetemperature));
	logger.print(tag, String("\n\t sensorId=") + String(sensorId));
	logger.print(tag, String("\n\t target=") + String(target));
	logger.print(tag, String("\n\t program=") + String(program));
	logger.print(tag, String("\n\t timerange=") + String(timerange));*/
	
	
	setSensorRemote(sensorRemote, sensorId);
	float temp = remotetemperature;
	//logger.print(tag, String("\n\t x1 remotetemperature=") + String(temp));
	setRemote(temp,lastTempUpdate);
	//logger.print(tag, String("\n\t x2 remotetemperature=") + String(temp));
	setTargetTemperature(target);

	if (currentStatus == Program::STATUS_DISABLED) {
		if (command.equals(command_enabled)) {
			enableRele(false);
			setStatus(Program::STATUS_IDLE);
		}
	}
	else {

		if (command.equals(command_ManualEnd)) { // se è in  manuale metti in idle

			enableRele(false);
			setStatus(Program::STATUS_IDLE);
		}
		else if (command.equals(command_Manual)) {

			logger.print(tag, F("\n\t manual "));
			enableRele(true);
			programDuration = duration;
			programStartTime = millis();
			setStatus(Program::STATUS_MANUAL_AUTO);

		}
		else if (command.equals(command_ManualOff)) {

			logger.print(tag, F("\n\t manual off"));
			enableRele(false);
			programDuration = duration;
			programStartTime = millis();
			setStatus(Program::STATUS_MANUAL_OFF);

		}
		else if (command.equals(command_ProgramOn)) {
			if (currentStatus != Program::STATUS_MANUAL_AUTO && currentStatus != Program::STATUS_MANUAL_OFF) {
				logger.print(tag, F("\n\t rele on not manual "));
				programDuration = duration;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
				enableRele(true);
				setStatus(Program::STATUS_PROGRAMACTIVE);
			}
		}
		else if (command.equals(command_ProgramOff)) {

			logger.print(tag, F("\n\t rele program off "));
			if (currentStatus != Program::STATUS_MANUAL_AUTO || currentStatus != Program::STATUS_MANUAL_OFF) {
			
				logger.print(tag, F("\n\t program active rele off "));
				enableRele(false);
				setStatus(Program::STATUS_PROGRAMACTIVE);
				programDuration = duration;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
			}

		}
		else if (command.equals(command_disabled)) {
			enableRele(false);
			setStatus(Program::STATUS_DISABLED);
		}
	}
	logger.print(tag, F("\n\t <<HeaterActuator::changeProgram"));
}

String HeaterActuator::getSensorAddress()
{
	return address;
}


void HeaterActuator::setStatus(int status)
{
	currentStatus = status;
}

int HeaterActuator::getStatus()
{
	return currentStatus;
}
String HeaterActuator::getStatusName()
{
	String name = statusStr[currentStatus];
	return name;
}

void HeaterActuator::setReleStatus(int status)
{
	releStatus = status;
}

int HeaterActuator::getReleStatus()
{
	return releStatus;
}

time_t HeaterActuator::getRemaininTime() {

	time_t remaining = programDuration - (millis() - programStartTime);
	return remaining;
}

time_t HeaterActuator::getProgramDuration()
{
	return programDuration;
}

int HeaterActuator::getActiveProgram()
{
	return activeProgram;
}

int HeaterActuator::getActiveTimeRange()
{
	return activeTimerange;
}

void HeaterActuator::enableRele(boolean on) {

	logger.print(tag, F("\n\t >>enableRele "));

/*	if (releStatus) {
		totalConsumptionTime += (millis() - lastConsumptionEnableTime);
	}

	oldReleStatus = releStatus;
	if (on) {
		logger.print(tag, F("\n\t enableRele:: RELE ON"));
		digitalWrite(Shield::heaterPin, RELE_ON);
		releStatus = true;

		lastConsumptionEnableTime = millis();
	}
	else {
		logger.print(tag, F("\n\t enableRele:: RELE OFF"));
		digitalWrite(Shield::heaterPin, RELE_OFF);
		releStatus = false;
	}*/
	logger.print(tag, F("\n\t <<enableRele "));
}

bool HeaterActuator::statusChanged() {
	if (oldCurrentStatus != currentStatus)
		return true;
	return false;
}

bool HeaterActuator::releStatusChanged() {
	if (oldReleStatus != releStatus)
		return true;
	return false;
}

void  HeaterActuator::saveOldStatus() {
	oldCurrentStatus = currentStatus;
}

void  HeaterActuator::saveOldReleStatus() {
	oldReleStatus = releStatus;
}

