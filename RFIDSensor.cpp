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
*/


//#define SS_PIN D8
//#define RST_PIN D3
//MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


//extern bool mqtt_publish(String topic, String message);


Logger RFIDSensor::logger;
String RFIDSensor::tag = "RFIDSensor";

#ifdef dopo
bool RFIDSensor::getJSON(JSONObject *jObject)
{
	//logger.print(tag, "\n");
	//logger.println(tag, ">>DoorSensor::getJSON");

	bool res = Sensor::getJSON(jObject);
	if (!res) return false;

	//res = jObject->pushBool("open", openStatus);

	//logger.println(tag, "<<DoorSensor::getJSON");
	return true;
}
#endif

String RFIDSensor::getJSONFields() {

	//logger.println(tag, ">>DoorSensor::getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	json += String(",\"card\":\"");
	json += String(lastcard);
	json += String("\"");
	/*if (openStatus)
		json += String(",\"open\":true");
	else
		json += String(",\"open\":false");*/

		//logger.println(tag, "<<DoorSensor::getJSONFields");
	return json;
}

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

/*bool DoorSensor::getOpenStatus() {
	return openStatus;
}*/

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

