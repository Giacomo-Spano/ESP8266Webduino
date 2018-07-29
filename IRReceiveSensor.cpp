#include "IRReceiveSensor.h"
#include "Util.h"
#include "Shield.h"


#include "ESP8266Webduino.h"

#ifdef ESP8266
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif




extern bool mqtt_publish(String topic, String message);

Logger IRReceiveSensor::logger;
String IRReceiveSensor::tag = "IRReceiveSensor";
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
#define RECV_PIN D4//14
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define CAPTURE_BUFFER_SIZE 1024

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
//#define RECV_PIN 14

// The Serial connection baud rate.
// i.e. Status message will be sent to the PC at this baud rate.
// Try to avoid slow speeds like 9600, as you will miss messages and
// cause other problems. 115200 (or faster) is recommended.
// NOTE: Make sure you set your Serial Monitor to the same speed.
#define BAUD_RATE 115200

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define CAPTURE_BUFFER_SIZE 1024

// TIMEOUT is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best TIMEOUT value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed MAX_TIMEOUT_MS. Typically 130ms.
#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
// e.g. Kelvinator
// A value this large may swallow repeats of some protocols
#else  // DECODE_AC
#define TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
#endif  // DECODE_AC
// Alternatives:
// #define TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
// units, but can accidentally swallow repeated messages
// in the rawData[] output.
// #define TIMEOUT MAX_TIMEOUT_MS  // This will set it to our currently allowed
// maximum. Values this high are problematic
// because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and
//   start sending it to serial at precisely
//   the time when the next message is likely
//   to be transmitted, and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the TIMEOUT value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
#define MIN_UNKNOWN_SIZE 12
// ==================== end of TUNEABLE PARAMETERS ====================

IRReceiveSensor::IRReceiveSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id, pin, enabled, address, name)
{
	type = "irreceivesensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;
}

IRReceiveSensor::~IRReceiveSensor()
{
}

#ifdef ESP8266

String IRReceiveSensor::getJSONFields() {

	//logger.println(tag, ">>IRSensor::getJSONFields");
	String json = "";
	json += Sensor::getJSONFields();

	// specific field
	json += String(",\"codetype\":\"") + codetype + String("\"");
	json += String(",\"code\":\"") + code + String("\"");
	json += String(",\"bit\":") + bit;

	codetype = "";
	code = "";
	bit = "0";
	//status = STATUS_IDLE;
	
	//logger.println(tag, "<<IRSensor::getJSONFields");
	return json;
}


void IRReceiveSensor::init()
{
	logger.print(tag, "\n\t >>init IRReceiveSensor");

#ifdef ESP8266

#if DECODE_HASH
	// Ignore messages with less than minimum on or off pulses.
	pirrecv = new IRrecv(pin, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
	pirrecv->setUnknownThreshold(MIN_UNKNOWN_SIZE);
#endif  // DECODE_HASH
	//pirrecv->enableIRIn();  // Start the receiver

#endif

	logger.print(tag, "\n\t <<init IRReceiveSensor");
}

bool IRReceiveSensor::checkStatusChange() {
	
	if (status == STATUS_RESPONSERECEIVEONEIRCODE) {
		logger.print(tag, "\n\t STATUS_RESPONSERECEIVEONEIRCODE");
		sendCommandResponse(receivedcommanduuid, getJSON());
		status = STATUS_IDLE;
		return false;
	} else if (status == STATUS_RECEIVEDIRCODE) {
		logger.print(tag, "\n\t STATUS_RECEIVEDIRCODE");

		logger.print(tag, "\n\t START RECEIVING AGAIN");

		/*pirrecv->enableIRIn();  // Start the receiver
		codetype = "";
		code += "";
		bit += "0";*/
		startMillis = millis();
		status = STATUS_RECEIVINGIRCODE;
		return true;
	}

	if (status == STATUS_RECEIVINGIRCODE || status == STATUS_RECEIVONEIRCODE) {
		receive();
	}

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

bool IRReceiveSensor::receiveCommand(String command, int id, String uuid, String json)
{
	logger.print(tag, "\n\t >>IRREceiveSensor::receiveCommand=");
	bool res = Sensor::receiveCommand(command, id, uuid, json);

	if (command.equals("send")) {
		logger.print(tag, "\n\t send command");
		receivedcommanduuid = uuid;

		pirrecv->enableIRIn();  // Start the receiver
		codetype = "";
		code += "";
		bit += "0";
		startMillis = millis();
		status = STATUS_RECEIVINGIRCODE;	
	}
	logger.print(tag, "\n\t <<IRREceiveSensor::receiveCommand=");
	return res;
}

void IRReceiveSensor::receive() {

	unsigned long currMillis = millis();
	if (status == STATUS_RECEIVONEIRCODE && currMillis - startMillis > 10000) {
		logger.print(tag, "\n\tTIMEOUT--*---");
		codetype = "";
		code += "";
		bit += "0";
		pirrecv->disableIRIn();
		status = STATUS_IDLE;
	}


	//wdt_enable(WDTO_8S);

	// Check if the IR code has been received.
	if (pirrecv->decode(&results)) {
		// Display a crude timestamp.
		uint32_t now = millis();
		Serial.printf("Timestamp : %06u.%03u\n", now / 1000, now % 1000);
		if (results.overflow)
			Serial.printf("WARNING: IR _code is too big for buffer (>= %d). "
				"This result shouldn't be trusted until this is resolved. "
				"Edit & increase CAPTURE_BUFFER_SIZE.\n",
				CAPTURE_BUFFER_SIZE);
		// Display the basic output of what we found.
		Serial.print(resultToHumanReadableBasic(&results));
		dumpACInfo(&results);  // Display any extra A/C info if we have it.
		yield();  // Feed the WDT as the text output can take a while to print.

				  // Display the library version the message was captured with.
		Serial.print("Library   : v");
		Serial.println(_IRREMOTEESP8266_VERSION_);
		Serial.println();

		// Output RAW timing info of the result.
		Serial.println(resultToTimingInfo(&results));
		yield();  // Feed the WDT (again)

				  // Output the results as source code
		Serial.println(resultToSourceCode(&results));
		Serial.println("");  // Blank line between entries
		yield();  // Feed the WDT (again)

		code = "";
		codetype = "";
		bit = "";
		codetype = typeToString(results.decode_type, results.repeat);
		if (hasACState(results.decode_type)) {
#if DECODE_AC
			for (uint16_t i = 0; results.bits > i * 8; i++) {
				if (results.state[i] < 0x10)  code += "0";  // Zero pad
				code += uint64ToString(results.state[i], 16);
			}
#endif  // DECODE_AC
		}
		else {
			code += uint64ToString(results.value, 16);
		}
		bit += uint64ToString(results.bits);

		Serial.println("codetype: " + codetype);
		Serial.println("code: " + code);
		Serial.println("bit: " + bit);

		
		if (status == STATUS_RECEIVONEIRCODE) {
			pirrecv->disableIRIn();
			status = STATUS_RESPONSERECEIVEONEIRCODE;
		}
		else {
			status = STATUS_RECEIVEDIRCODE;
		}
	}
}



// Display the human readable state of an A/C message if we can.
void IRReceiveSensor::dumpACInfo(decode_results *results) {
	String description = "";
#if DECODE_DAIKIN
	if (results->decode_type == DAIKIN) {
		IRDaikinESP ac(0);
		ac.setRaw(results->state);
		description = ac.toString();
	}
#endif  // DECODE_DAIKIN
#if DECODE_FUJITSU_AC
	if (results->decode_type == FUJITSU_AC) {
		IRFujitsuAC ac(0);
		ac.setRaw(results->state, results->bits / 8);
		description = ac.toString();
	}
#endif  // DECODE_FUJITSU_AC
#if DECODE_KELVINATOR
	if (results->decode_type == KELVINATOR) {
		IRKelvinatorAC ac(0);
		ac.setRaw(results->state);
		description = ac.toString();
	}
#endif  // DECODE_KELVINATOR
#if DECODE_TOSHIBA_AC
	if (results->decode_type == TOSHIBA_AC) {
		IRToshibaAC ac(0);
		ac.setRaw(results->state);
		description = ac.toString();
	}
#endif  // DECODE_TOSHIBA_AC
#if DECODE_GREE
	if (results->decode_type == GREE) {
		IRGreeAC ac(0);
		ac.setRaw(results->state);
		description = ac.toString();
	}
#endif  // DECODE_GREE
#if DECODE_MIDEA
	if (results->decode_type == MIDEA) {
		IRMideaAC ac(0);
		ac.setRaw(results->value);  // Midea uses value instead of state.
		description = ac.toString();
	}
#endif  // DECODE_MIDEA
#if DECODE_HAIER_AC
	if (results->decode_type == HAIER_AC) {
		IRHaierAC ac(0);
		ac.setRaw(results->state);
		description = ac.toString();
	}
#endif  // DECODE_HAIER_AC
	// If we got a human-readable description of the message, display it.
	if (description != "")  Serial.println("Mesg Desc.: " + description);
}
#endif