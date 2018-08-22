#include "RFIDSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

/*

Signal	Pin
RST/Reset	D3
SS/SDA		D8
MOSI		D7
MISO		D6
SCK			D5
IRQ			DISCONNECTED
3.3V		3.3
*/




Logger RFIDSensor::logger;
String RFIDSensor::tag = "RFIDSensor";

/*JsonObject& RFIDSensor::getJson() {
	logger.print(tag, F("\n\t >>RFIDSensor::getJson"));

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	Sensor::getJson(json);

	//JsonObject& json = Sensor::getJson();
	json["card"] = lastcard;
	logger.printJson(json);
	logger.print(tag, F("\n\t <<RFIDSensor::getJson"));
	return json;
}*/

void RFIDSensor::getJson(JsonObject& json) {

	logger.print(tag, "\n\t >>RFIDSensor::getJsonx");
	Sensor::getJson(json);
	json["card"] = lastcard;
	logger.printJson(json);

	logger.print(tag, "\n\t <<RFIDSensor::getJsonx");
}

/*String RFIDSensor::getJSONFields() {

	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	json += String(",\"card\":\"");
	json += String(lastcard);
	json += String("\"");
	return json;
}*/

RFIDSensor::RFIDSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	type = "RFIDSensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	lastCardRead = 0;
	cardRead_interval = 3000;
}

RFIDSensor::~RFIDSensor()
{
}

void RFIDSensor::init()
{
	logger.print(tag, "\n\t >>init RFIDSensor");

	SPI.begin();      // Initiate  SPI bus
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();   // Initiate MFRC522
	logger.print(tag, "\n\t <<init RFIDSensor");
}

bool RFIDSensor::receiveCommand(String command, int id, String uuid, String jsonStr)
{
	logger.print(tag, "\n\t >>RFIDSensor::receiveCommand");
	bool res = Sensor::receiveCommand(command, id, uuid, jsonStr);
	logger.print(tag, "\n\t <<RFIDSensor::receiveCommand=");
	return res;
}

bool RFIDSensor::checkStatusChange() {

	lastcard = "";
	//logger.print(tag, "\n\t >>checkrfidStatus: ");
	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCardRead;
	if (enabled && timeDiff > cardRead_interval/*timeDiff > checkStatus_interval*/) {
		//logger.print(tag, "\n\t >>checkDoorStatus: ");
		lastCheckStatus = currMillis;

		// Look for new cards
		if (!mfrc522->PICC_IsNewCardPresent())
		{
			return false;
		}
		// Select one of the cards
		if (!mfrc522->PICC_ReadCardSerial())
		{
			return false;
		}
		//Show UID on serial monitor
		Serial.print("UID tag :");
		//String content = "";
		//lastcard = "";
		byte letter;
		for (byte i = 0; i < mfrc522->uid.size; i++)
		{
			Serial.print(mfrc522->uid.uidByte[i] < 0x10 ? " 0" : " ");
			Serial.print(mfrc522->uid.uidByte[i], HEX);
			lastcard.concat(String(mfrc522->uid.uidByte[i] < 0x10 ? " 0" : " "));
			lastcard.concat(String(mfrc522->uid.uidByte[i], HEX));
		}
		Serial.println();
		Serial.print("Message : ");
		lastcard.toUpperCase();
		Serial.print("card: ");
		Serial.println(lastcard);
		/*if (lastcard.substring(1) == "76 FD 97 BB") //change here the UID of the card/cards that you want to give access
		{
			Serial.println("Authorized access");
			Serial.println();
			delay(3000);
		}

		else {
			Serial.println(" Access denied");
			delay(3000);
		}*/
		lastCardRead = millis();
		return true;
	}
	return false;
}

