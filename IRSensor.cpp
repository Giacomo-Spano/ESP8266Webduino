#include "IRSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"
#include <IRremoteESP8266.h>

#include "DoorSensor.h"
#include "Util.h"
#include "ESP8266Webduino.h"
#include "Shield.h"

extern bool mqtt_publish(String topic, String message);

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
	
	pirsend = new IRsend(pin);
	pirsend->begin();
	
	pdaikinir = new IRDaikinESP(pin);
	pdaikinir->begin();
	
	
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


bool IRSensor::receiveCommand(String command, int id, String uuid, String json)
{
	bool res = Sensor::receiveCommand(command, id, uuid, json);
	logger.println(tag, ">>receiveCommand=");
	logger.print(tag, "\n\t command=" + command);
	//int SAMSUNG_BITS = 32;
	
	if (command.equals("send")) {
		logger.print(tag, "\n\t send command");

		
		sendHarmanKardonDisc();
		sendSamsungTv();
		//sendRobotClean();
		//sendRobotHome();
		//sendDaikin();
		
	}

	logger.println(tag, "<<receiveCommand res="/* + String(res)*/);
	return res;
}

bool IRSensor::sendSamsungTv() {
	Serial.println("sendSamsungTv");
	#define SAMSUNG_POWER_ON  GenNECXCode(7,7,153)
	#define SAMSUNG_Channel_Up  GenNECXCode(7,7,18)
	pirsend->sendSAMSUNG(SAMSUNG_Channel_Up, 32);
	delayMicroseconds(50);
	return true;
}

bool IRSensor::sendDaikin() {

	Serial.println("sendDaikin");

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

	return true;

}

uint64_t clean_data = 0x2AA22DD;
uint16_t clean_rawData[62] = { 484, 626,  510, 598,  514, 602,  514, 596,  514, 626,  1614, 628,  492, 622,  1636, 596,  514, 602,  1634, 628,  488, 626,  1632, 614,  504, 598,  1640, 620,  494, 622,  496, 618,  520, 586,  1638, 602,  510, 628,  492, 628,  478, 626,  1638, 594,  530, 600,  1630, 626,  1614, 626,  498, 624,  1628, 604,  1626, 628,  1626, 622,  480, 624,  1634, 602 };  // UNKNOWN 7E5AA54A
uint64_t home_data = 0x2AA8877;
uint16_t home_rawData[62] = { 514, 622,  488, 626,  484, 626,  488, 622,  510, 600,  1634, 626,  494, 628,  1610, 626,  506, 616,  1624, 596,  514, 626,  1612, 638,  474, 634,  1640, 600,  514, 600,  1634, 628,  490, 620,  498, 628,  504, 590,  1638, 598,  516, 624,  496, 628,  484, 622,  502, 626,  1618, 610,  1624, 626,  1624, 620,  506, 610,  1628, 606,  1628, 626,  1612, 626 };  // UNKNOWN 3DA514


uint16_t HK_DISC_rawData[40] = { 510, 610,  510, 610,  510, 606,  1630, 610,  514, 606,  1630, 610,  1630, 606,  1630, 610,  1632, 610,  1630, 606,  1644, 598,  514, 604,  1638, 598,  514, 608,  514, 594,  45110, 9062,  2210, 598,  65535, 0,  30655, 9070,  2218, 600 };  // UNKNOWN 25B83BF
uint16_t HK_AVR_rawData[48] = { 1630, 604,  1634, 606,  514, 606,  514, 606,  514, 604,  514, 606,  510, 610,  510, 606,  514, 606,  1634, 606,  1634, 606,  1642, 600,  1628, 618,  1622, 620,  1616, 610,  1632, 604,  1644, 596,  514, 610,  512, 590,  45106, 9072,  2204, 584,  65535, 0,  30677, 9064,  2218, 576 };  // UNKNOWN FDC03324
uint16_t HK_OFF_rawData[140] = { 514, 606,  510, 606,  514, 606,  514, 606,  514, 606,  514, 606,  514, 618,  1618, 606,  514, 606,  528, 592,  514, 606,  514, 606,  1630, 604,  1648, 602,  1622, 620,  512, 610,  1624, 598,  1642, 608,  1620, 624,  1620, 604,  1638, 602,  514, 606,  526, 600,  1624, 606,  514, 606,  516, 604,  520, 600,  516, 606,  508, 614,  1630, 604,  1640, 598,  528, 566,  45126, 9054,  2228, 562,  36842, 9056,  4458, 610,  510, 606,  516, 618,  502, 602,  518, 610,  502, 616,  504, 606,  518, 612,  1624, 608,  510, 620,  512, 596,  512, 606,  512, 612,  1624, 606,  1642, 612,  1624, 602,  514, 622,  1622, 598,  1634, 614,  1622, 610,  1632, 608,  1628, 614,  524, 592,  514, 606,  1630, 606,  528, 604,  510, 606,  516, 594,  518, 600,  518, 602,  1634, 600,  1646, 594,  518, 562,  45128, 9076,  2216, 560 };  // UNKNOWN 98D9F3F8
uint16_t harmapoweronProntoCode[76] = {
	0x0000, 0x006e, 0x0022, 0x0002, 0x0154, 0x00ac, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0040, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0041, 0x0015, 0x0041, 0x0015, 0x0040, 0x0015, 0x0015, 0x0015, 0x0040, 0x0015, 0x0040, 0x0015, 0x0041, 0x0015, 0x0041, 0x0015, 0x0041, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0040, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0015, 0x0040, 0x0015, 0x0040, 0x0015, 0x0015, 0x0015, 0x06af, 0x0154, 0x0056, 0x0015, 0x0e58
};

bool IRSensor::sendRobotClean() {

	Serial.println("sendVaacumCleaner");

	// robot aspirapolvere. Manda due codici di seguito 

	
	Serial.println("NEC");
	pirsend->sendNEC(clean_data/*0x2AA22DD*/, 32);
	delayMicroseconds(50);

	pirsend->sendRaw(clean_rawData, 62, 38);  // Send a raw data capture at 38kHz.
	delay(15000);

	return true;
}

bool IRSensor::sendRobotHome() {

	Serial.println("sendVaacumCleaner");

	// robot aspirapolvere. Manda due codici di seguito 


	Serial.println("NEC");
	pirsend->sendNEC(home_data/*0x2AA22DD*/, 32);
	delayMicroseconds(50);

	pirsend->sendRaw(home_rawData, 62, 38);  // Send a raw data capture at 38kHz.
	delay(15000);

	return true;
}

bool IRSensor::sendHarmanKardonDisc() {

	Serial.println("sendHarmanKardonDisc");
		
	pirsend->sendPronto(harmapoweronProntoCode, 76);
	//pirsend->sendRaw(HK_DISC_rawData, 40, 38);  // Send a raw data capture at 38kHz.
	delay(15000);

	return true;
}
