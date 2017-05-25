#include "HeaterSensor.h"
#include "Logger.h"
#include "Command.h"

Logger HeaterSensor::logger;
String HeaterSensor::tag = "HeaterSensor";

//char* HeaterSensor::statusStr[] = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };

HeaterSensor::HeaterSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	checkStatus_interval = 10000;
	lastCheckStatus = 0;
	type = "heatersensor";
}

HeaterSensor::~HeaterSensor()
{
}

CommandResponse HeaterSensor::receiveCommand(String jsonStr)
{
	logger.println(tag, ">>receiveCommand=");

	JSON json(jsonStr);

	// actuatorId
	int actuatorId;
	if (json.has("actuatorid")) {
		actuatorId = json.jsonGetInt("actuatorid");
		logger.print(tag, "\n\t actuatorid=" + String(actuatorId));
	}

	// command
	String command = "";
	if (json.has("command")) {
		command = json.jsonGetString("command");
		logger.print(tag, "\n\t command=" + command);
	}
	// duration
	int duration = 0;
	if (json.has("duration")) {
		duration = json.jsonGetInt("duration");
		logger.print(tag, "\n\t duration=" + String(duration) + " minuti");
		duration = duration * 60 * 1000;
		logger.print(tag, "\n\t duration=" + String(duration) + " millisecondi");
	}
	// remote temperature
	float remoteTemperature = 0;
	if (json.has("temperature")) {
		remoteTemperature = json.jsonGetFloat("temperature");
		logger.print(tag, "\n\t temperature=" + String(remoteTemperature));
	}
	// target
	float target = 0;
	if (json.has("target")) {
		target = json.jsonGetFloat("target");
		logger.print(tag, "\n\t target=" + String(target));
	}
	// scenario
	int scenario = 0;
	if (json.has("scenario")) {
		scenario = json.jsonGetInt("scenario");
		logger.print(tag, "\n\t scenario=" + String(scenario));
	}
	// scenario time interval
	int timeInterval = 0;
	if (json.has("timeinterval")) {
		timeInterval = json.jsonGetInt("timeinterval");
		logger.print(tag, "\n\t timeinterval=" + String(timeInterval));
	}
	// sensor
	int zone = 0;
	if (json.has("sensor")) {
		zone = json.jsonGetInt("zone");
		logger.print(tag, "\n\t sensorId=" + String(zone));
	}


	logger.print(tag, "\n\t changeProgram param=" + String(remoteTemperature));
	bool res = changeStatus(command, duration,
		remoteTemperature,
		target,
		scenario,
		timeInterval,
		zone);
	CommandResponse response;

	logger.println(tag, "<<receiveCommand res=" + String(res));
	return response;
}

String HeaterSensor::getJSONFields()
{
	logger.print(tag, "\n\t >>HeaterActuator::getJSONFields");

	String json = "";
	json += Sensor::getJSONFields();


	json += ",\"status\":\"" + status + "\"";
	json += ",\"relestatus\":" + String((getReleStatus()) ? "true" : "false");

	if (status.equals(STATUS_KEEPTEMPERATURE)) {

		json += ",\"remotetemperature\":" + String(getRemoteTemperature());

		json += ",\"duration\":";
		json += String(programDuration / 1000);

		int remainingTime = programDuration - (millis() - programStartTime);
		json += ",\"remaining\":";
		json += String(remainingTime / 1000);

		json += ",\"target\":";
		json += String(getTargetTemperature());

		json += ",\"scenario\":";
		json += String(activescenario);

		json += ",\"timeinterval\":";
		json += String(activeTimeInterval);
	}
	logger.print(tag, "\n\t <<HeaterActuator::getJSONFields json=" + json);
	return json;
}

bool HeaterSensor::getJSON(JSONObject * jObject)
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


void HeaterSensor::init()
{
	logger.println(tag, "\n\t init");
	ConsumptionStartTime = 0;// millis();
	pinMode(pin, OUTPUT);
	setStatus(STATUS_OFF);
	//sensorname = "Riscaldamento";
	logger.println(tag, "\n\t init");
}

bool HeaterSensor::checkStatusChange()
{
	//logger.print(tag, "\n\n\t >>HeaterActuator::checkStatusChange");

	bool sendStatus = false;

	// controlla se il programma attivo è finito.
	if (programEnded()) {
		sendStatus = true;
		logger.print(tag, "\n\n\t HEATER::PROGRAM END \n\n");
	}


	// notifica il server se è cambiato lo stato del rele
	if (releStatusChanged()) {

		logger.println(tag, "\t!->rele status changed");
		saveOldReleStatus();
		sendStatus = true;

		logger.print(tag, "\n\n\t HEATER::RELE STATUS CHANGED \n\n");
	}

	// notifica il server se è cambiato lo status
	if (statusChanged()) {

		logger.print(tag, "\n\n\t HEATER::STATUS CHANGED \n\n");
		saveOldStatus();
		sendStatus = true;
	}

	return sendStatus;
}

void HeaterSensor::setTargetTemperature(float target)
{
	logger.print(tag, "\n\t >>setTargetTemperature " + String(target));
	targetTemperature = target;
}

float HeaterSensor::getTargetTemperature() {
	return targetTemperature;
}

void HeaterSensor::setRemoteTemperature(float temp)
{
	logger.print(tag, "\n\t remoteTemperature" + String(remoteTemperature));
	remoteTemperature = temp;
	last_RemoteSensor = millis();
	logger.print(tag, ",last_RemoteSensor=" + String(last_RemoteSensor));
}

float HeaterSensor::getRemoteTemperature() {
	return remoteTemperature;
}

bool HeaterSensor::programEnded()
{
	unsigned long currMillis = millis();

	if (status.equals(STATUS_KEEPTEMPERATURE)) {

		/*if (sensorRemote && (currMillis - last_RemoteSensor) > remoteSensorTimeout) {
			// ferma il programma se è passato troppo tempo dall'ultimo aggiornamento ricevuto dal server
			logger.println(tag, "REMOTE SENSOR TIMEOUT");
			// è inutile mandare un sendstatus perchè tanto cambia lo stato dopo e verrebbe inviato due volte
			remoteTemperature = 0;
			setStatus(Program::STATUS_IDLE);
			return true;
		}
		else */
		if (currMillis - programStartTime > programDuration) {
			logger.println(tag, "END PROGRAM");
			// � iniutile mandare un sendstatus perch� tanto cambia lo stato dopo e verrebbe inviato due volte
			setStatus(STATUS_OFF);
			updateReleStatus();
			//enableRele(false);
			return true;
		}
	}
	return false;
}

void HeaterSensor::updateReleStatus() {

	logger.print(tag, "\n\n\t>>updateReleStatus");

	logger.print(tag, "\n\t ConsumptionStartTime=" + String(ConsumptionStartTime));
	logger.print(tag, "\n\t totalConsumptionTime=" + String(totalConsumptionTime));
	logger.print(tag, "\n\t lastConsumptionEnableTime=" + String(lastConsumptionEnableTime));
	logger.print(tag, "\n\t targetTemperature=" + String(targetTemperature));
	logger.print(tag, "\n\t scenario=" + String(activescenario));
	logger.print(tag, "\n\t timeinterval=" + String(activeTimeInterval));
	logger.print(tag, "\n\t zoneid=" + String(zoneId));
	logger.print(tag, "\n\t remoteTemperature=" + String(remoteTemperature));
	logger.print(tag, "\n\n");

	if (!enabled || status.equals(STATUS_OFF)) {

		logger.print(tag, "\n\t disabled");
		enableRele(false);

	}
	else if (status.equals(STATUS_KEEPTEMPERATURE)) {
		logger.print(tag, "\n\t STATUS OFF");
		enableRele(false);
	}
	else if (status.equals(STATUS_KEEPTEMPERATURE)) {

		logger.print(tag, "\n\t STATUS_KEEPTEMPERATURE");

		if (remoteTemperature < targetTemperature) {

			logger.print(tag, F("-LOW TEMPERATURE"));
			enableRele(true);
		}
		else {
			logger.print(tag, F("-HIGH TEMPERATURE"));
			enableRele(false);
		}
	}
	logger.print(tag, "\n\t <<updateReleStatus\n");
}

bool HeaterSensor::changeStatus(String command, long duration, float remotetemperature, float target, int scenario, int timeinterval, int zone) {

	logger.print(tag, F("\n\t >>HeaterActuator::changeStatus"));

	logger.print(tag, "\n\t status=" + status);
	logger.print(tag, String("\n\t command=") + command);
	logger.print(tag, String("\n\t duration=") + String(duration));
	logger.print(tag, String("\n\t remotetemperature=") + String(remotetemperature));
	logger.print(tag, String("\n\t target=") + String(target));
	logger.print(tag, String("\n\t scenario=") + String(scenario));
	logger.print(tag, String("\n\t timeinterval=") + String(timeinterval));
	logger.print(tag, String("\n\t zone=") + String(zone));


	if (!enabled) {
		return false;
	}
	if (command.equals(command_KeepTemperature)) {

		logger.print(tag, F("\n\t keeptemperature "));
		setRemoteTemperature(remoteTemperature);
		setTargetTemperature(target);
		setStatus(STATUS_KEEPTEMPERATURE);
		programDuration = duration;
		programStartTime = millis();

	}
	else if (command.equals(command_Off)) {

		logger.print(tag, F("\n\t off "));
		setStatus(STATUS_OFF);
		enableRele(false);

	}
	else if (command.equals(command_sendTempareture)) {

		logger.print(tag, F("\n\t off "));
		setRemoteTemperature(remoteTemperature);
		enableRele(false);

	}
	updateReleStatus();
	logger.print(tag, F("\n\t <<HeaterActuator::changeStatus"));
}

void HeaterSensor::setStatus(String status)
{
	oldStatus = status;
	this->status = status;
}

String HeaterSensor::getStatus()
{
	return status;
}

void HeaterSensor::setReleStatus(int status)
{
	releStatus = status;
}

int HeaterSensor::getReleStatus()
{
	return releStatus;
}

time_t HeaterSensor::getRemaininTime() {

	time_t remaining = programDuration - (millis() - programStartTime);
	return remaining;
}

void HeaterSensor::enableRele(boolean on) {

	logger.print(tag, F("\n\t >>enableRele "));

	if (releStatus) {
		totalConsumptionTime += (millis() - lastConsumptionEnableTime);
	}

	oldReleStatus = releStatus;
	if (on) {
		logger.print(tag, F("\n\t enableRele:: RELE ON"));
		digitalWrite(pin, RELE_ON);
		releStatus = true;

		lastConsumptionEnableTime = millis();
	}
	else {
		logger.print(tag, F("\n\t enableRele:: RELE OFF"));
		digitalWrite(pin, RELE_OFF);
		releStatus = false;
	}
	logger.print(tag, F("\n\t <<enableRele "));
}

bool HeaterSensor::statusChanged() {
	if (!oldStatus.equals(status))
		return true;
	return false;
}

bool HeaterSensor::releStatusChanged() {
	if (oldReleStatus != releStatus)
		return true;
	return false;
}

void  HeaterSensor::saveOldStatus() {
	oldStatus = status;
}

void  HeaterSensor::saveOldReleStatus() {
	oldReleStatus = releStatus;
}

