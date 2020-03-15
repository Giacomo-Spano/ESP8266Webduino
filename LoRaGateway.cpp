#include "LoRaGateway.h"

#include <SPI.h>
#include <LoRa.h>
#include "Logger.h"
#include "MQTTMessage.h"


Logger LoRaGateway::logger;
String LoRaGateway::tag = "LoRaGateway";


extern bool mqtt_publish(MQTTMessage mqttmessage);
extern bool mqtt_subscribe(String topic);
//extern void messageReceived(String topic, String message);
//extern void messageReceived(char* topic, byte* payload, unsigned int length);


//define the pins used by the transceiver module
//#ifndef TTGO
/*#define ss 5
#define rst 14
#define dio0 2*/
//#endif

//#ifdef TTGO
#define ss 18
#define rst 14
#define dio0 26
//#endif

/*#define ss 18//5
#define rst 14
#define dio0 26//2*/

/*int ss = 5;//5
int rst = 14;
int dio0 = 2;//2*/

int counter = 0;

/**************/
//const int csPin = 7;          // LoRa radio chip select
//const int resetPin = 6;       // LoRa radio reset
//const int irqPin = 1;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
/**/

LoRaGateway::LoRaGateway()
{
}

LoRaGateway::~LoRaGateway()
{
}

bool LoRaGateway::getGatewayServer() {
	return gatewayServer;
}

String LoRaGateway::getTargetAddress() {
	return targetAddress;
}

void LoRaGateway::init(String address, bool serverenabled, ESPDisplay* pDisplay)
{
	//espDisplay = pDisplay;
	//initialize Serial Monitor
	Serial.begin(115200);

	logger.print(tag, F("\n\t LoRaGateway::init"));
	//Serial.println("LoRa Sender");

	logger.print(tag, F("\n\t address="));
	logger.print(tag, address);
	targetAddress = address;

	gatewayServer = serverenabled;
	logger.print(tag, F("\n\t gatewayServer="));
	logger.print(tag, Logger::boolToString(gatewayServer));

	
	//setup LoRa transceiver module
	LoRa.setPins(ss, rst, dio0);

	//replace the LoRa.begin(---E-) argument with your location's frequency 
	//433E6 for Asia
	//866E6 for Europe
	//915E6 for North America
	logger.print(tag, F("\n\t initializing."));
	while (!LoRa.begin(866E6)) {
		logger.print(tag, F("."));
		delay(500);
	}
	// Change sync word (0xF3) to match the receiver
	// The sync word assures you don't get LoRa messages from other LoRa transceivers
	// ranges from 0-0xFF
	LoRa.setSyncWord(0xF3);
	logger.print(tag, F("\n\t LoRa Initializing OK!"));
}

void LoRaGateway::senderloop() {
	logger.print(tag, F("\n\t Sending packet: "));
	logger.print(tag, counter);

	//Send LoRa packet to receiver
	String msg = "messaggio " + counter;
	sendLoraMessage(msg);
	/*LoRa.beginPacket();
	LoRa.print("hello ");
	LoRa.print(counter);
	LoRa.endPacket();
	counter++;*/
}

void LoRaGateway::sendLoraMessage(String outgoing) {
	logger.print(tag, F("\n\tsendLoraMessage"));
	LoRa.beginPacket();                   // start packet
	LoRa.write(destination);              // add destination address
	LoRa.write(localAddress);             // add sender address
	LoRa.write(msgCount);                 // add message ID
	LoRa.write(outgoing.length());        // add payload length
	LoRa.print(outgoing);                 // add payload
	LoRa.endPacket();                     // finish packet and send it
	counter++;                           // increment message ID
}

// parse for a packet, and call onReceive with the result:

String LoRaGateway::receiverloop() { // questa funzione viene chiamata in continuazione 
								// sia per il gateway server che per il gateway client
								// - se è un server inserisce il messaggio nella coda MQTT
								// - se è un client chiama la funzione messagerecaive che gestisce i messaggio 
								// normalmente ricevuti dalla coda MQTT
	
	int packetSize = LoRa.parsePacket();
	if (packetSize == 0) 
		return "";          // if there's no packet, return
	else {

		String LoRaData = "";
		while (LoRa.available()) {
			LoRaData += LoRa.readString();
			Serial.print(LoRaData);
		}

		// print RSSI of packet
		Serial.print("' with RSSI ");
		Serial.println(LoRa.packetRssi());
		logger.print(tag, "\n\t data: " + LoRaData);

		MQTTMessage mqttmessage2;
		mqttmessage2.topic = "lora/prova";
		mqttmessage2.message = LoRaData;

		mqtt_publish(mqttmessage2);
		return LoRaData;
	}


	logger.print(tag, "\n\n\t LoRaGateway::receiverloop -->LoRa packet received");
	// read packet header bytes:
	int recipient = LoRa.read();          // recipient address
	byte sender = LoRa.read();            // sender address
	byte incomingMsgId = LoRa.read();     // incoming msg ID
	byte incomingLength = LoRa.read();    // incoming msg length
	String incoming = "";

	while (LoRa.available()) {
		incoming += (char)LoRa.read();
	}

	if (incomingLength != incoming.length()) {   // check length for error
		logger.print(tag, F("\n\t error: message length does not match length"));
		return "";                             // skip rest of function
	}

	// if the recipient isn't this device or broadcast,
	if (recipient != localAddress && recipient != 0xFF) {
		logger.print(tag, F("\n\t This message is not for me."));
		return "";                             // skip rest of function
	}

	// if message is for this device, or broadcast, print details:
	logger.print(tag, "\n\t Received from: 0x" + String(sender, HEX));
	logger.print(tag, "\n\t Sent to: 0x" + String(recipient, HEX));
	logger.print(tag, "\n\t Message ID: " + String(incomingMsgId));
	logger.print(tag, "\n\t Message length: " + String(incomingLength));
	logger.print(tag, "\n\t Message: " + incoming);
	logger.print(tag, "\n\t RSSI: " + String(LoRa.packetRssi()));
	logger.print(tag, "\n\t Snr: " + String(LoRa.packetSnr()));
	String ret = "l:" + String(incomingLength) +
		/*",mID:" + String(incomingMsgId) +*/
		",RSSI:" + String(LoRa.packetRssi()) +
		",Snr:" + String(LoRa.packetSnr());

	int l = incoming.indexOf(";");
	int len = incoming.substring(0, l).toInt();
	String topic = incoming.substring(l + 1, l + 1 + len);
	topic += ""; // sen<a questo la parse messagge non riconosce il messaggio. C'è qualche problema con il fine stringa
	//logger.print(tag, "\n\t topic len: " + String(len));
	//logger.print(tag, "\n\t topic: " + topic);

	String message = incoming.substring(l + 2 + len, incoming.length());
	//logger.print(tag, "\n\t message: " + message);
	l = incoming.indexOf(";");
	len = message.substring(0, l).toInt();
	message = message.substring(l + 1, incoming.length());
	//logger.print(tag, "\n\t message len: " + String(len));
	//logger.print(tag, "\n\t message: " + message);
	logger.print(tag, "\n\t topic: " + topic);
	logger.print(tag, "\n\t message: " + message);

	MQTTMessage mqttmessage;
	mqttmessage.topic = topic;
	mqttmessage.message = message;

	if (gatewayServer) {	 // se è un gateway server pubblica su MQTT messaggio proveniente 
							// da client LoRa
		mqtt_publish(mqttmessage);
	}
	else { //altrimenti interpreta il messaggio proveniente dal server
		// manda un messaggio alla funz messagereceived come se arrivasse da MQTT
		//messageReceived(mqttmessage.topic, mqttmessage.message);
	}

	logger.print(tag, "\n\t LoRaGateway::receiverloop <--LoRa packet received" + ret);

	return ret;
}
