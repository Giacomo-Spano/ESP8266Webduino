#include "HeaterSensor.h"
#include "Logger.h"
#include "Command.h"

#include "ESP8266Webduino.h"

Logger HeaterSensor::logger;
String HeaterSensor::tag = "HeaterSensor";
//gger HeaterSensor::status = "";

//char* HeaterSensor::statusStr[] = { "unused", "idle", "program", "manual", "disabled", "restarted", "manualoff" };

HeaterSensor::HeaterSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	checkStatus_interval = 10000;
	lastCheckStatus = 0;
	type = "heatersensor";
	setStatus(STATUS_IDLE);
}

HeaterSensor::~HeaterSensor()
{
}

bool HeaterSensor::receiveCommand(String command, int id, String uuid, String jsonStr)
{
	logger.print(tag, "\n\t >>HeaterSensor::receiveCommand=" + command);
	bool res = Sensor::receiveCommand(command, id, uuid, jsonStr);
	if (res) {// se è true vuol dire che è stato inviato il comando base
		logger.print(tag, "\n\t <<HeaterSensor::receiveCommand res=" + Logger::boolToString(res));
		return true;
	}
		
	size_t size = jsonStr.length();
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(jsonStr);
	// duration
	int duration = 0;
	if (json.containsKey("duration")) {
		duration = json["duration"];
		logger.print(tag, "\n\t duration=" + String(duration) + " secondi");
	}
	// remote temperature
	float rTemperature = 0;
	if (json.containsKey("temperature")) {
		rTemperature = json["temperature"];
		logger.print(tag, "\n\t temperature=" + String(rTemperature));
	}
	// target
	float target = 0;
	if (json.containsKey("target")) {
		target = json["target"];
		logger.print(tag, "\n\t target=" + String(target));
	}
	// actionid
	int actionid = 0;
	if (json.containsKey("actionid")) {
		actionid = json["actionid"];
		logger.print(tag, "\n\t actionid=" + String(actionid));
	}
	// commanddate
	String commanddate = "";
	if (json.containsKey("date")) {
		String str = json["date"];
		commanddate = str;
		logger.print(tag, "\n\t date=" + commanddate);
	}
	// enddate
	String enddate = "";
	if (json.containsKey("enddate")) {
		String str = json["enddate"];
		enddate = str;
		logger.print(tag, "\n\t enddate=" + enddate);
	}
	// zone
	int zone = 0;
	if (json.containsKey("zoneid")) {
		zone = json["zoneid"];
		logger.print(tag, "\n\t zoneid=" + String(zoneid));
	}
	// zonesensor
	int zonesensorid = 0;
	if (json.containsKey("zonesensorid")) {
		zone = json["zonesensorid"];
		logger.print(tag, "\n\t zonesensorid=" + String(zonesensorid));
	}

	logger.print(tag, "\n\t changeProgram param=" + String(rTemperature));
	res = changeStatus(command, duration,
		rTemperature,
		target,
		actionid,
		commanddate,
		enddate,
		zoneid,
		zonesensorid);
	if (res) {
		logger.print(tag, F("\n\t command executed - send response"));
		DynamicJsonBuffer jsonBuffer;
		JsonObject& jsonresult = jsonBuffer.createObject();
		getJson(jsonresult);
		String jsonStr;
		logger.printJson(jsonresult);
		jsonresult.printTo(jsonStr);
		logger.print(tag, F("\n\t jsonstr="));
		logger.print(tag, jsonStr);
		sendCommandResponse(uuid, jsonStr);
	}

	logger.print(tag, "\n\t <<HeaterSensor::receiveCommand res=" + Logger::boolToString(res));
	return res;
}


void HeaterSensor::getJson(JsonObject& json) {

	Sensor::getJson(json);
	json["relestatus"] = getReleStatus();
	if (status.equals(STATUS_KEEPTEMPERATURE)) {

		json["remotetemp"] = getRemoteTemperature();
		json["duration"] = programDuration;
		json["remaining"] = getRemaininTime();
		json["target"] = getTargetTemperature();
		json["actionid"] = activeActionId;
		json["zoneid"] = zoneid;
		json["zonesensorid"] = zonesensorid;
		json["enddate"] = endDate;
	}
	json["lastcmnd"] = lastCommandDate;
	json["lasttemp"] = lastTemperatureUpdate;
}

#ifdef dopo
String HeaterSensor::getJSONFields()
{
	//logger.print(tag, "\n\t >>HeaterActuator::getJSONFields");

	String json = "";
	json += Sensor::getJSONFields();


	//json += ",\"status\":\"" + status + "\"";
	json += ",\"relestatus\":" + String((getReleStatus()) ? "true" : "false");

	if (status.equals(STATUS_KEEPTEMPERATURE)) {

		json += ",\"remotetemp\":" + String(getRemoteTemperature());

		json += ",\"duration\":";
		json += String(programDuration);

		int remainingTime = getRemaininTime();// -(millis() - programStartTime);
		json += ",\"remaining\":";
		json += String(remainingTime);

		json += ",\"target\":";
		json += String(getTargetTemperature());

		json += ",\"actionid\":";
		json += String(activeActionId);

		json += ",\"zoneid\":";
		json += String(zoneId);

		json += ",\"enddate\":\"";
		json += String(endDate);
		json += "\"";
	} else if (status.equals(STATUS_MANUAL)) {

		json += ",\"remotetemp\":" + String(getRemoteTemperature());

		json += ",\"duration\":";
		json += String(programDuration);

		int remainingTime = getRemaininTime();//programDuration - (millis() - programStartTime);
		json += ",\"remaining\":";
		json += String(remainingTime);

		json += ",\"target\":";
		json += String(getTargetTemperature());

		json += ",\"zoneid\":";
		json += String(zoneId);

		json += ",\"enddate\":\"";
		json += String(endDate);
		json += "\"";
	}
	json += ",\"lastcmnd\":\"";
	json += String(lastCommandDate);
	json += "\"";

	json += ",\"lasttemp\":\"";
	json += String(lastTemperatureUpdate);
	json += "\"";

	//logger.print(tag, "\n\t <<HeaterActuator::getJSONFields json=" + json);
	return json;
}
#endif

void HeaterSensor::init()
{
	logger.println(tag, "\n\t init");
	ConsumptionStartTime = 0;// millis();
	pinMode(pin, OUTPUT);
	//setStatus(STATUS_OFF);
	//sensorname = "Riscaldamento";
	logger.println(tag, "\n\t init");
}

bool HeaterSensor::checkStatusChange()
{
	bool sendStatus = false;

	// controlla se è mpassato un minuto
	if (status.equals(STATUS_KEEPTEMPERATURE)) {
		//logger.print(tag, "\n\n\t HEATER::REMAINING TIME :  " + String(getRemaininTime()));
		
		remainingSeconds = getRemaininTime();
		if (remainingSeconds != oldRemainingSeconds && remainingSeconds % 10 == 0) {
			oldRemainingSeconds = remainingSeconds;
			//sendStatus = true;
			logger.print(tag, "\n\n\t HEATER STATUS " + status + " REMAINING SECONDS :  " + String(remainingSeconds));
			logger.print(tag, " (" + String(remainingSeconds / 60) + ":" + String(remainingSeconds % 60) + ")");
			logger.print(tag, "\n");
		}
	}

	// controlla se il programma attivo è finito.
	if (programEnded()) {
		sendStatus = true;
		logger.print(tag, "\n\n\t HEATER::PROGRAM END \n\n");
	}


	// notifica il server se è cambiato lo stato del rele
	if (releStatusChanged()) {

		//logger.println(tag, "\t!->rele status changed");
		saveOldReleStatus();
		sendStatus = true;

		logger.print(tag, "\n\n\t HEATER::RELE STATUS CHANGED \n\n");
	}

	// notifica il server se è cambiato lo status
	if (statusChanged()) {

		logger.print(tag, "\n\n\t HEATER::STATUS CHANGED \n\n");
		oldStatus = status;
		sendStatus = true;
	}

	return sendStatus;
}

void HeaterSensor::setTargetTemperature(float target)
{
	logger.print(tag, "\n\t >>HeaterSensor::setTargetTemperature " + String(target));
	targetTemperature = target;
}

float HeaterSensor::getTargetTemperature() {
	return targetTemperature;
}

void HeaterSensor::setRemoteTemperature(float rtemp)
{
	logger.print(tag, "\n\t HeaterSensor::setRemoteTemperature rtemp=" + String(rtemp));
	remoteTemperature = rtemp;
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

		// ferma il programma se è passato troppo tempo dall'ultimo aggiornamento ricevuto dal server
		if (currMillis - programStartTime > 1000 * programDuration) { 
			logger.println(tag, "END PROGRAM");
			// � iniutile mandare un sendstatus perch� tanto cambia lo stato dopo e verrebbe inviato due volte
			setStatus(STATUS_IDLE);
			updateReleStatus();
			enableRele(false);
			return true;
		} 
	}
	return false;
}

#ifdef dopo
bool HeaterSensor::remoteTemperatureUpdate()
{
	
	if (status.equals(STATUS_MANUAL)) {
		
		unsigned long currMillis = millis();
		if (currMillis - last_RemoteSensor > remoteTemperatureTimeout) {
			logger.println(tag, "REMOTE TEMPERATURE TIMEOUT");
			setStatus(STATUS_OFF);
			updateReleStatus();
			//enableRele(false);
			return true;
		}

		if (currMillis - last_RemoteSensor > remoteTemperatureRefreshTimeout) {
			logger.println(tag, "REMOTE TEMPERATURE REFRESH TIMEOUT");
			requestZoneTemperatureUpdate();
			return false;
		}
	}
	return false;
}
#endif

void HeaterSensor::updateReleStatus() {

	logger.print(tag, "\n\n\t >>updateReleStatus" + String(remoteTemperature));

	/*logger.print(tag, "\n\t ConsumptionStartTime=" + String(ConsumptionStartTime));
	logger.print(tag, "\n\t totalConsumptionTime=" + String(totalConsumptionTime));
	logger.print(tag, "\n\t lastConsumptionEnableTime=" + String(lastConsumptionEnableTime));
	logger.print(tag, "\n\t targetTemperature=" + String(targetTemperature));
	logger.print(tag, "\n\t scenario=" + String(activescenario));
	logger.print(tag, "\n\t timeinterval=" + String(activeTimeInterval));
	logger.print(tag, "\n\t scenario=" + String(activeActionId));
	logger.print(tag, "\n\t timeinterval=" + lastCommandDate);
	logger.print(tag, "\n\t zoneid=" + String(zoneid));
	logger.print(tag, "\n\t zonesensorid=" + String(zonesensorid));
	logger.print(tag, "\n\t remoteTemperature=" + String(remoteTemperature));
	logger.print(tag, "\n\n");*/

	if (!enabled || status.equals(STATUS_IDLE) || status.equals(STATUS_OFF)) {

		logger.print(tag, "\n\t disabled");
		enableRele(false);

	}
	else if (status.equals(STATUS_KEEPTEMPERATURE)) {

		if (remoteTemperature < targetTemperature) {

			logger.print(tag, F("STATUS_KEEPTEMPERATURE-LOW TEMPERATURE"));
			enableRele(true);
		}
		else {
			logger.print(tag, F("STATUS_KEEPTEMPERATURE-HIGH TEMPERATURE"));
			enableRele(false);
		}
	}
	logger.print(tag, "\n\t <<updateReleStatus\n");
}

bool HeaterSensor::changeStatus(String command, long duration, float rtemp, float target, int actionid, String commanddate,String enddate, int zoneid, int zonesensorid) {

	logger.print(tag, F("\n\t >>HeaterSensor::changeStatus"));
	/*logger.print(tag, String("\n\t current status=") + status);
	logger.print(tag, String("\n\t command=") + command);
	logger.print(tag, String("\n\t duration=") + String(duration));
	logger.print(tag, String("\n\t remotetemperature=") + String(rtemp));
	logger.print(tag, String("\n\t target=") + String(target));
	logger.print(tag, String("\n\t actionid=") + String(actionid));
	logger.print(tag, String("\n\t commanddate=") + String(commanddate));
	logger.print(tag, String("\n\t enddate=") + String(enddate));
	logger.print(tag, String("\n\t zoneid=") + String(zoneid));
	logger.print(tag, String("\n\t zonesensorid=") + String(zonesensorid));*/

	if (!enabled) {
		logger.print(tag, F("\n\t sensor not enabled "));
		return false;
	}
	if (command.equals(command_KeepTemperature)) {

		logger.print(tag, F("\n\t command keeptemperature "));
		setRemoteTemperature(rtemp);
		lastTemperatureUpdate = commanddate;
		setTargetTemperature(target);
		setStatus(STATUS_KEEPTEMPERATURE);
		programDuration = duration;
		programStartTime = millis();
		activeActionId = actionid;
		setZone(zoneid, zonesensorid);
		lastCommandDate = commanddate;
		endDate = enddate;

	}
	else if (command.equals(command_Stop_KeepTemperature)) {

		logger.print(tag, F("\n\t command stop keeptemperature "));
		enableRele(false);
		setStatus(STATUS_IDLE);
		lastCommandDate = commanddate;
		endDate = "";
		
	}
	else if (command.equals(command_Idle)) {

		logger.print(tag, F("\n\t command idle "));
		enableRele(false);
		setStatus(STATUS_IDLE);
		lastCommandDate = commanddate;
		endDate = "";

	}
	else if (command.equals(command_Off)) {

		logger.print(tag, F("\n\t command off "));
		setStatus(STATUS_OFF);
		enableRele(false);
		lastCommandDate = commanddate;
		endDate = "";
	}
	else if (command.equals(command_sendTemperature)) {

		logger.print(tag, F("\n\t command sendtemperature "));
		logger.print(tag, String("\n\t remotetemperature=") + String(rtemp));
		setRemoteTemperature(rtemp);
		lastTemperatureUpdate = commanddate;
		//enableRele(false);
		lastCommandDate = commanddate;

	}
	else {
		//logger.print(tag, F("\n\t invalid command "));
		logger.print(tag, F("\n\t <<HeaterSensor::changeStatus failed"));
		return false;
	}

	logger.print(tag, String(F("\n\t --HeaterSensor::changeStatus status=")) + status);
	logger.print(tag, " oldStatus=" + oldStatus);

	updateReleStatus();

	//logger.print(tag, "\n\t --HeaterSensor::changeStatus status=" + status);
	logger.print(tag, " oldStatus=" + oldStatus);

	logger.print(tag, F("\n\t <<HeaterSensor::changeStatus"));
	return true;
}

void HeaterSensor::setStatus(String sensorstatus)
{
	logger.print(tag, "\n\t >>HeaterSensor::setStatus " + sensorstatus);
	//oldStatus = status;
	logger.print(tag, "\n\t oldStatus=" + oldStatus);
	status = sensorstatus;
	//logger.print(tag, F("\n\t <<HeaterSensor::setStatus"));

	logger.print(tag, "\n\t <<HeaterSensor::setStatus=" + status);
}

void HeaterSensor::setZone(int _zoneid, int _zonesensorid)
{
	logger.print(tag, "\n\t >>HeaterSensor::setZone " + String(zoneid + " _zonesensorid=" + zonesensorid));
	zoneid = _zoneid;
	zonesensorid = _zonesensorid;
	logger.print(tag, "\n\t <<HeaterSensor::setZone " + String(zoneid + " _zonesensorid=" + zonesensorid));
}

String HeaterSensor::getStatus()
{
	return status;
}

void HeaterSensor::setReleStatus(bool rstatus)
{
	releStatus = rstatus;
}

bool HeaterSensor::getReleStatus()
{
	return releStatus;
}

time_t HeaterSensor::getRemaininTime() { // tempo rimanente in secondi

	time_t remaining = programDuration - (millis() - programStartTime) / 1000;
	return remaining;
}

void HeaterSensor::enableRele(bool on) {

	logger.print(tag, F("\n\t >>enableRele "));

	if (releStatus) {
		totalConsumptionTime += (millis() - lastConsumptionEnableTime);
	}

	oldReleStatus = releStatus;
	if (on) {
		logger.print(tag, F("\n\t RELE ON"));
		digitalWrite(pin, RELE_ON);
		releStatus = true;

		lastConsumptionEnableTime = millis();
	}
	else {
		logger.print(tag, F("\n\t RELE OFF"));
		digitalWrite(pin, RELE_OFF);
		releStatus = false;
	}
	logger.print(tag, F("\n\t <<enableRele "));
}

bool HeaterSensor::statusChanged() {
	if (!oldStatus.equals(status)) {
		//logger.print(tag, "\n\t HeaterSensor::statusChanged oldstatus=" + oldStatus);
		//logger.print(tag, " newstatus=" + status);
		return true;
	}
	return false;
}

bool HeaterSensor::releStatusChanged() {
	if (oldReleStatus != releStatus) {
		//logger.print(tag, "\n\t HeaterSensor::statusChanged oldReleStatus=" + String(oldReleStatus));
		//logger.print(tag, " releStatus=" + String(releStatus));
		return true;
	}
	return false;
}

void  HeaterSensor::saveOldReleStatus() {
	oldReleStatus = releStatus;
}

#ifdef dopo
bool HeaterSensor::requestZoneTemperatureUpdate(int shieldid)
{
	logger.print(tag, "\n");
	logger.println(tag, ">> requestZoneTemperatureUpdate");

	Command command;
	logger.print(tag, "\t REQUEST ZONE TEMPERATURE\n");

	String json = "{";
	json += "\"shieldid\":" + String(shieldid);// shieldid
	json += ",\"id\":" + String(sensorid);// shieldid
	json += ",\"zoneid\":" + String(zoneId);
	json += "}";

	bool res = command.requestZoneTemperature(json);
	logger.println(tag, "<< requestZoneTemperatureUpdate\n");

	return res;
}
#endif

