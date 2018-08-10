#include "IRSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

#ifdef ESP8266
#include <IRremoteESP8266.h>
#endif

#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

//extern bool mqtt_publish(String topic, String message);

Logger IRSensor::logger;
String IRSensor::tag = "IRSensor";

IRSensor::IRSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	type = "irsensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;
}

IRSensor::~IRSensor()
{
}

String IRSensor::getJSONFields() {

	//logger.println(tag, ">>IRSensor::getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();
	// specific field
	//logger.println(tag, "<<IRSensor::getJSONFields");
	return json;
}

void IRSensor::init()
{
	logger.print(tag, "\n\t >>init IRSensor");
	
#ifdef ESP8266
	pirsend = new IRsend(pin);
	pirsend->begin();
	
	pdaikinir = new IRDaikinESP(pin);
	pdaikinir->begin();
#endif // ESP8266	
	
	logger.print(tag, "\n\t <<init IRSensor");
}

bool IRSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	if (timeDiff > checkStatus_interval) {
		//logger.print(tag, "\n\t >>>> checkStatusChange - timeDiff > checkStatus_interval");
		/*lastCheckStatus = currMillis;
		String oldStatus = status;

		if (digitalRead(pin) == LOW) {
			status = STATUS_DOOROPEN;
		}
		else {
			status = STATUS_DOORCLOSED;
		}

		if (!status.equals(oldStatus)) {
			if (status.equals(STATUS_DOOROPEN))
				logger.print(tag, "\n\t >>>> DOOR OPEN");
			else if (status.equals(STATUS_DOORCLOSED))
				logger.print(tag, "\n\t >>>> DOOR CLOSED");
			return true;
		}*/
	}
	return false;
}

// Reverse the order of bits in a byte
// Example: 01000000 -> 00000010
unsigned char ReverseByte(unsigned char b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return(b);
}

// Calculate 32 bit NECx code
unsigned long GenNECXCode(unsigned char p_Device, unsigned char p_SubDevice, unsigned char p_Function)
{
	unsigned long ReverseDevice = (unsigned long)ReverseByte(p_Device);
	unsigned long ReverseSubDevice = (unsigned long)ReverseByte(p_SubDevice);
	unsigned long ReverseFunction = (unsigned long)ReverseByte(p_Function);
	return((ReverseDevice << 24) | (ReverseSubDevice << 16) | (ReverseFunction << 8) | ((~ReverseFunction) & 0xFF));
}

uint64_t StrToHex(const char* str)
{
	return (uint64_t)strtoull(str, 0, 16);
}

bool IRSensor::receiveCommand(String command, int id, String uuid, String jsonStr)
{
	logger.print(tag, "\n\t >>IRSensor::receiveCommand=");
	bool res = Sensor::receiveCommand(command, id, uuid, jsonStr);
	//logger.print(tag, "\n\t command=" + command);
	//int SAMSUNG_BITS = 32;
	
	if (command.equals("send")) {
		logger.print(tag, "\n\t send command");

		String codetype, code;
		uint64_t value;
		int bit;


		size_t size = jsonStr.length();
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(jsonStr);

		//JSON json(jsonStr);
		if (json.containsKey("codetype")) {
			String str = json["codetype"];
			codetype = str;
			logger.print(tag, "\n\t codetype=" + codetype);
		}
		else {
			logger.print(tag, "\n\t error");
			logger.print(tag, "\n\t <<IRSensor::receiveCommand=");
			return false;
		}
		if (json.containsKey("code")) {
			String str = json["code"];
			code = str;
			logger.print(tag, "\n\t code=" + code);

			//code = "0x" + code;
			while (code.length() < 16)
				code = "0" + code;
			logger.print(tag, "\n\t code=" + code);

			uint8_t copy[16];
			for (int i = 0; i < 16; i++) {
				copy[i] = code.charAt(i);
			}

			value = StrToHex(code.c_str());
		}
		else {
			logger.print(tag, "\n\t error");
			logger.print(tag, "\n\t <<IRSensor::receiveCommand=");
			return false;
		}
		if (json.containsKey("bit")) {
			bit = json["bit"];
			logger.print(tag, "\n\t bit=" + String(bit));
		}
		else {
			logger.print(tag, "\n\t error");
			logger.print(tag, "\n\t <<IRSensor::receiveCommand=");
			return false;
		}
				
		sendCode(codetype, value, bit);
		//sendHarmanKardonDisc();
		//sendSamsungTv();
		//sendRobotClean();
		//sendRobotHome();
		//sendDaikin();
		
	}
	logger.print(tag, "\n\t <<IRSensor::receiveCommand=");
	return res;
}



bool IRSensor::sendCode(String codetype, uint64_t code, int bit) {

	logger.print(tag, "\n\t >>sendCode");
#ifdef ESP8266

	pirsend->sendNEC(code, bit);  // Send a raw data capture at 38kHz.
									//pirsend->sendPronto(harmapoweronProntoCode, 76);
									//pirsend->sendRaw(HK_DISC_rawData, 40, 19);  // Send a raw data capture at 38kHz.
									//delay(15000);
#endif
	logger.print(tag, "\n\t <<sendCode");
	return true;
}


bool IRSensor::sendSamsungTv() {

#ifdef ESP8266
	Serial.println("sendSamsungTv");
	#define SAMSUNG_POWER_ON  GenNECXCode(7,7,153)
	#define SAMSUNG_Channel_Up  GenNECXCode(7,7,18)
	pirsend->sendSAMSUNG(SAMSUNG_Channel_Up, 32);
	delayMicroseconds(50);
	Serial.println("SamsungTv sent");
#endif
	return true;
}

bool IRSensor::sendDaikin() {

	Serial.println("sendDaikin");
#ifdef ESP8266
	// Set up what we want to send. See ir_Daikin.cpp for all the options.
	pdaikinir->on();
	pdaikinir->setFan(1);
	pdaikinir->setMode(DAIKIN_COOL);
	pdaikinir->setTemp(25);
	pdaikinir->setSwingVertical(false);
	pdaikinir->setSwingHorizontal(false);

	// Set the current time to 1:33PM (13:33)
	// Time works in minutes past midnight
	//daikinir.setCurrentTime((13 * 60) + 33);
	
	// Turn off about 1 hour later at 2:30PM (15:30)
	//daikinir.enableOffTimer((14 * 60) + 30);

	// Display what we are going to send.
	Serial.println(pdaikinir->toString());

	// Now send the IR signal.
	pdaikinir->send();

	delay(15000);
#endif
	return true;

}

uint64_t clean_data = 0x2AA22DD;
uint16_t clean_rawData[62] = { 484, 626,  510, 598,  514, 602,  514, 596,  514, 626,  1614, 628,  492, 622,  1636, 596,  514, 602,  1634, 628,  488, 626,  1632, 614,  504, 598,  1640, 620,  494, 622,  496, 618,  520, 586,  1638, 602,  510, 628,  492, 628,  478, 626,  1638, 594,  530, 600,  1630, 626,  1614, 626,  498, 624,  1628, 604,  1626, 628,  1626, 622,  480, 624,  1634, 602 };  // UNKNOWN 7E5AA54A
uint64_t home_data = 0x2AA8877;
uint16_t home_rawData[62] = { 514, 622,  488, 626,  484, 626,  488, 622,  510, 600,  1634, 626,  494, 628,  1610, 626,  506, 616,  1624, 596,  514, 626,  1612, 638,  474, 634,  1640, 600,  514, 600,  1634, 628,  490, 620,  498, 628,  504, 590,  1638, 598,  516, 624,  496, 628,  484, 622,  502, 626,  1618, 610,  1624, 626,  1624, 620,  506, 610,  1628, 606,  1628, 626,  1612, 626 };  // UNKNOWN 3DA514


uint64_t ROBOT_HOME = 0x2AA8877;
uint64_t ROBOT_CLEAN = 0x2AA22DD;



uint64_t HK_DISC = 0x10E0BF4;
uint64_t HK_TV = 0x10E8D72;
uint64_t HK_SERVER = 0x10ECD32;
uint64_t HK_AUDIO = 0x10E2DD2;
uint64_t HK_AVR = 0x10E03FC;
uint64_t HK_ON = 0x10E03FC;
uint64_t HK_OFF = 0x10EF906;
uint64_t HK_VOLUP = 0x10EE31C;
uint64_t HK_VOLDOWN = 0x10E13EC;
uint64_t HK_MUTE = 0x10E837C;


bool IRSensor::sendRobotClean() {

	Serial.println("sendVaacumCleaner");
#ifdef ESP8266
	// robot aspirapolvere. Manda due codici di seguito 

	
	Serial.println("NEC");
	pirsend->sendNEC(clean_data/*0x2AA22DD*/, 32);
	delayMicroseconds(50);

	pirsend->sendRaw(clean_rawData, 62, 38);  // Send a raw data capture at 38kHz.
	delay(15000);
#endif
	return true;
}

bool IRSensor::sendRobotHome() {

	Serial.println("sendVaacumCleaner");
#ifdef ESP8266
	// robot aspirapolvere. Manda due codici di seguito 


	Serial.println("NEC");
	//pirsend->sendNEC(home_data/*0x2AA22DD*/, 32);
	pirsend->sendNEC(ROBOT_HOME, 32);
	delayMicroseconds(50);

	pirsend->sendRaw(home_rawData, 62, 38);  // Send a raw data capture at 38kHz.
	delay(15000);
#endif
	return true;
}

bool IRSensor::sendHarmanKardonDisc() {

	Serial.println("sendHarmanKardonDisc");
#ifdef ESP8266

	pirsend->sendNEC(HK_DISC, 32);  // Send a raw data capture at 38kHz.
	//pirsend->sendPronto(harmapoweronProntoCode, 76);
	//pirsend->sendRaw(HK_DISC_rawData, 40, 19);  // Send a raw data capture at 38kHz.
	//delay(15000);
#endif
	Serial.println("HarmanKardonDisc sent");
	return true;
}
