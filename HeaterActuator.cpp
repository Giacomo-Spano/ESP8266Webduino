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

void HeaterActuator::checkStatus()
{

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
		/*if (getStatus() == Program::STATUS_DISABLED) {
			command.sendActuatorStatus(*this);
		}*/
		saveOldStatus();
		sendStatus = true;
	}
		
	if (sendStatus) {
		Command command;
		command.sendActuatorStatus(*this);
	}
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
	logger.print(tag, F("\n\tsetSensorRemote"));
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
	logger.print(tag, "\n\n");

	if (currentStatus == Program::STATUS_MANUAL_OFF) {

		logger.print(tag, "\n\tSTATUS_MANUAL_OFF");
		enableRele(false);
	}
	else if (currentStatus == Program::STATUS_MANUAL_AUTO) {

		logger.print(tag, "\n\tSTATUS_MANUAL_AUTO");
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
		logger.print(tag, "\n\tINACTIVE-rele OFF");
		enableRele(false);
	}

	logger.print(tag, "\n\t<<updateReleStatus\n");
}

void HeaterActuator::changeProgram(int command, long duration, bool sensorRemote, float remotetemperature, int sensorId, float target, int program, int timerange) {

	logger.print(tag, F("\t>>changeProgram"));

	logger.print(tag, F("\n\tcurrentStatus="));
	logger.print(tag, currentStatus);

	setSensorRemote(sensorRemote, sensorId);
	setRemoteTemperature(remoteTemperature);

	if (currentStatus == Program::STATUS_DISABLED) {

		if (command == command_enabled) {
			enableRele(false);
			setStatus(Program::STATUS_IDLE);
		}

	}
	else {

		if (command == command_ManualEnd) { // se è in  manuale metti in idle

			enableRele(false);
			setStatus(Program::STATUS_IDLE);
		}
		else if (command == command_Manual) {

			logger.print(tag, F("\n\tmanual "));
			enableRele(true);
			programDuration = duration;
			programStartTime = millis();
			setStatus(Program::STATUS_MANUAL_AUTO);

		}
		else if (command == command_ManualOff) {

			logger.print(tag, F("\n\tmanual off"));
			enableRele(false);
			programDuration = duration;
			programStartTime = millis();
			setStatus(Program::STATUS_MANUAL_OFF);

		}
		else if (command == command_ProgramOn) {

			if (currentStatus != Program::STATUS_MANUAL_AUTO && currentStatus != Program::STATUS_MANUAL_OFF) {

				logger.print(tag, F("\n\trele on not manual "));
				programDuration = duration;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
				enableRele(true);
				setStatus(Program::STATUS_PROGRAMACTIVE);

			}
		}
		else if (command == command_ProgramOff) {

			logger.print(tag, F("\n\trele program off "));
			if (currentStatus != Program::STATUS_MANUAL_AUTO || currentStatus != Program::STATUS_MANUAL_OFF) {
			
				logger.print(tag, F("\n\tprogram active rele off "));
				enableRele(false);
				setStatus(Program::STATUS_PROGRAMACTIVE);
				programDuration = duration;
				programStartTime = millis();
				activeProgram = program;
				activeTimerange = timerange;
			}

		}
		else if (command == command_disabled) {
			enableRele(false);
			setStatus(Program::STATUS_DISABLED);
		}
	}
	logger.println(tag, F("\t<<changeProgram"));
}

String HeaterActuator::getSensorAddress()
{
	return subaddress;
}

String HeaterActuator::getJSON() {
	String json = "";
	json += "{";

	//json += "\"event\":\"update\",";

	json += "\"shieldid\":" + String(Shield::id) + ",";
	json += "\"remotetemperature\":" + String(getRemoteTemperature()) + ",";
	json += "\"addr\":\"" + subaddress + "\",";
	json += "\"status\":\"" + String(statusStr[getStatus()]) + "\",";
	json += "\"type\":\"heater\",";
	json += "\"name\":\"" + sensorname + "\",";
	json += "\"relestatus\":\"" + String((getReleStatus()) ? "true" : "false") + "\"";
	//json += "\"duration\":" + String(getProgramDuration()/1000) + ",";
	//json += "\"remaining\":" + String(getRemaininTime()/1000) + "";

	if (getStatus() == Program::STATUS_PROGRAMACTIVE
		|| getStatus() == Program::STATUS_MANUAL_AUTO
		|| getStatus() == Program::STATUS_MANUAL_OFF) {

		json += F(",\"duration\":");
		json += String(programDuration / 1000);

		int remainingTime = programDuration - (millis() - programStartTime);
		json += String(F(",\"remaining\":"));
		json += String(remainingTime / 1000);

		json += F(",\"localsensor\":");
		if (!sensorIsRemote())
			json += F("true");
		else
			json += F("false");

		if (getStatus() == Program::STATUS_MANUAL_AUTO || getStatus() == Program::STATUS_PROGRAMACTIVE) {
			json += F(",\"target\":");
			json += String(getTargetTemperature());
		}

		if (getStatus() == Program::STATUS_PROGRAMACTIVE) {
			json += F(",\"program\":");
			json += String(getActiveProgram());

			json += F(",\"timerange\":");
			json += String(getActiveTimeRange());
		}

	}

	json += "}";


	return json;
}

void HeaterActuator::setStatus(int status)
{
	currentStatus = status;
	//updateReleStatus();
}

/*void HeaterActuator::setManualMode(int status, int mode)
{
	setStatus(status);
	manualMode = mode;
}*/

int HeaterActuator::getStatus()
{
	return currentStatus;
}
/*int HeaterActuator::getManualMode()
{
	return manualMode;
}*/

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

	logger.print(tag, F("\n\t>>enableRele "));
	//logger.print(tag, F(" on="));
	//logger.print(tag, String(on));

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
	logger.print(tag, F("\n\t<<enableRele "));
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

