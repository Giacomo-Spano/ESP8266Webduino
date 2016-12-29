#include "HeaterActuator.h"
#include "Logger.h"
#include "Command.h"

extern Logger logger;
extern const char* statusStr[];

HeaterActuator::HeaterActuator()
{
	tag = "HeaterActuator";
}

HeaterActuator::~HeaterActuator()
{
}

void HeaterActuator::init(String MACAddress)
{
	
	ConsumptionStartTime = millis();
	pinMode(relePin, OUTPUT);
	setStatus(Program::STATUS_IDLE);
	subaddress += MACAddress;

	sensorname = "Riscaldamento";

}

void HeaterActuator::setTargetTemperature(float target)
{
	logger.print(tag, F("\n\tsetTargetTemperature"));

	logger.print(tag, F(" targetTemperature="));
	logger.print(tag, targetTemperature);
	targetTemperature = target;
}

float HeaterActuator::getTargetTemperature() {
	return targetTemperature;
}

void HeaterActuator::setRemoteTemperature(float remote)
{
	logger.print(tag, F("\n\tsetRemoteTemperature"));
	logger.print(tag, F(" remote="));
	logger.print(tag, remote);

	remoteTemperature = remote;

	last_RemoteSensor = millis();
	logger.print(tag, F(",last_RemoteSensor="));
	logger.print(tag, last_RemoteSensor);

	/*if (sensorRemote)
		updateReleStatus();*/ // non serve. E' controllato da remoto'
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
	logger.print (tag, F("\n\tsetSensorRemote"));
	logger.print(tag, F(" tenable="));
	logger.print(tag, enable);
	logger.print(tag, F(",sensorId="));
	logger.print(tag, sensorId);

	sensorRemote = enable;
	remoteSensorId = sensorId;

	last_RemoteSensor = millis();
	logger.print(tag, F(",last_RemoteSensor="));
	logger.print(tag, last_RemoteSensor);
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

	if (currentStatus == Program::STATUS_PROGRAMACTIVE || currentStatus == Program::STATUS_MANUAL) {

		if (sensorRemote && (currMillis - last_RemoteSensor) > remoteSensorTimeout) {
			// ferma il programma se è passato troppo tempo dall'ultimo aggiornamento ricevuto dal server
			logger.println(tag, "REMOTE SENSOR TIMEOUT");
			// è inutile mandare un sendstatus perchè tanto cambia lo stato dopo e verrebbe inviato due volte
			remoteTemperature = 0;
			setStatus(Program::STATUS_IDLE);
			//enableRele(false);			
			return true;
		}
		else if (currMillis - programStartTime > programDuration) {
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

	logger.print(tag, "\n\tConsumptionStartTime=");
	logger.print(tag, String(ConsumptionStartTime));
	logger.print(tag, "\n\ttotalConsumptionTime=");
	logger.print(tag, String(totalConsumptionTime));
	logger.print(tag, "\n\tlastConsumptionEnableTime=");
	logger.print(tag, String(lastConsumptionEnableTime));
	logger.print(tag, "\n\ttargetTemperature=");
	logger.print(tag, String(targetTemperature));
	logger.print(tag, "\n\tsensorRemote=");
	logger.print(tag, String(sensorRemote));
	logger.print(tag, "\n\tremoteSensorId=");
	logger.print(tag, String(remoteSensorId));
	logger.print(tag, "\n\tremoteTemperature=");
	logger.print(tag, String(remoteTemperature));
	logger.print(tag, "\n\t");
		
	if (currentStatus == Program::STATUS_MANUAL) {

		logger.print(tag, "\n\tSTATUS_MANUAL");
		// se stato manuale accendi il rel� se sensore == locale e temperatura sensore locale < temperatura target oppure
		// se sensore == remoto e temperature sensore remoto < temperatura target 
		if (manualMode == HeaterActuator::MANUALMODE_OFF) {
			logger.print(tag, F("-MANUAL OFF"));
			enableRele(false);
		}
		else if ((!sensorRemote && localAvTemperature < targetTemperature) ||
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

		logger.print(tag, "\n\tSTATUS_PROGRAMACTIVE");

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
		logger.print(tag, "INACTIVE-rele OFF");
		enableRele(false);
	}
}

void HeaterActuator::changeProgram(int status, long duration, int manual, bool sensorRemote, float remotetemperature, int sensorId, float target, int program, int timerange, int localsensor) {

	logger.println(tag, F("changeProgram"));

	logger.print(tag, F("\n\tcurrentStatus="));
	logger.print(tag, currentStatus);
	
	setSensorRemote(sensorRemote, sensorId);
	setRemoteTemperature(remoteTemperature);

	if (status == relestatus_on && currentStatus != Program::STATUS_DISABLED) {

		if (manual == HeaterActuator::MANUALMODE_DISABLED) {
			logger.print(tag, F("\n\trele on "));
			if (currentStatus != Program::STATUS_MANUAL) {
				logger.print(tag, F("\n\tnot manual "));
				if (duration != -1)
					programDuration = duration;
				else
					programDuration = 30000;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
				enableRele(true);
				setStatus(Program::STATUS_PROGRAMACTIVE);				
			}
		}
		else if (manual == HeaterActuator::MANUALMODE_AUTO || manual == HeaterActuator::MANUALMODE_OFF) {
			logger.print(tag, F("\n\tmanual "));
			enableRele(true);			
			if (duration != -1)
				programDuration = duration;
			else
				programDuration = 30000;
			programStartTime = millis();
			//setStatus(Program::STATUS_MANUAL);
			setManualMode(Program::STATUS_MANUAL, manual/*HeaterActuator::MANUALMODE_AUTO*/);
		}
	}
	else if (status == relestatus_off) {
		logger.print(tag, F("\n\trele off "));
		if (currentStatus == Program::STATUS_MANUAL) { // il programma è finito ma il dispositivo è in manual mode
			if (manual == HeaterActuator::MANUALMODE_AUTO || manual == HeaterActuator::MANUALMODE_OFF) {
				logger.print(tag, F("\n\tmanual stop "));
				enableRele(false);
				setStatus(Program::STATUS_IDLE);
			}
			else {
				logger.print(tag, F("\n\tmanual mode, impossibile fermare"));
			}
		}
		else if (currentStatus == Program::STATUS_PROGRAMACTIVE) {
			logger.print(tag, F("\n\tprogram active rele off "));
			enableRele(false);
			activeProgram = program;
			activeTimerange = timerange;
		}
		else if (currentStatus == Program::STATUS_IDLE) {

			if (manual == HeaterActuator::MANUALMODE_DISABLED) {
				logger.print(tag, F("\n\trele off "));
				logger.print(tag, F("\n\tnot manual "));
				enableRele(false);
				setStatus(Program::STATUS_PROGRAMACTIVE);
				if (duration != -1)
					programDuration = duration;
				else
					programDuration = 30000;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
			}
		}
	}
	else if (status == relestatus_disabled) {
		enableRele(false);
		setStatus(Program::STATUS_DISABLED);
	}
	else if (status == relestatus_enabled) {
		enableRele(false);
		setStatus(Program::STATUS_IDLE);
	}
}

String HeaterActuator::getSensorAddress()
{
	return subaddress;
}

String HeaterActuator::getJSON() {
	String json = "";
	json += "{";

	json += "\"command\":\"status\",";
	//json += "\"id\":" + String(settings.id) + ",";
	json += "\"addr\":\"" + subaddress + "\",";
	json += "\"status\":\"" + String(statusStr[getStatus()]) + "\",";
	json += "\"type\":\"heater\",";
	json += "\"name\":\"" + sensorname + "\",";
	json += "\"relestatus\":\"" + String((getReleStatus()) ? "true" : "false") + "\",";
	json += "\"remaining\":" + String(getRemaininTime()) + "";
	json += "}";

	/*json += "\"temperature\":";
	json += Util::floatToString(temperature);
	json += ",\"avtemperature\":";
	json += Util::floatToString(avTemperature);
	json += ",\"name\":\"";
	json += String(sensorname) + "\"";
	json += ",\"type\":\"temperature\"";
	json += ",\"addr\":\"";
	json += String(getSensorAddress()) + "\"}";*/
	return json;
}

void HeaterActuator::setStatus(int status)
{
	currentStatus = status;
	//updateReleStatus();
}

void HeaterActuator::setManualMode(int status, int mode)
{
	setStatus(status);
	manualMode = mode;
}

int HeaterActuator::getStatus()
{
	return currentStatus;
}
int HeaterActuator::getManualMode()
{
	return manualMode;
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

int HeaterActuator::getActiveProgram()
{
	return activeProgram;
}

int HeaterActuator::getActiveTimeRange()
{
	return activeTimerange;
}

void HeaterActuator::enableRele(boolean on) {

	logger.print(tag, F("enableRele "));
	logger.print(tag, F(" on="));
	logger.print(tag, String(on));

	if (releStatus) {
		totalConsumptionTime += (millis() - lastConsumptionEnableTime);
	}

	oldReleStatus = releStatus;
	if (on) {
		logger.print(tag, F(" enableRele:: RELE ON"));
		digitalWrite(relePin, RELE_ON);
		releStatus = true;

		lastConsumptionEnableTime = millis();
	}
	else {
		logger.print(tag, F(" enableRele:: RELE OFF"));
		digitalWrite(relePin, RELE_OFF);
		releStatus = false;
	}

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

