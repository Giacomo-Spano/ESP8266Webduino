#ifndef _LORAGATEWAY_h
#define _LORAGATEWAY_h

#include <Arduino.h>
#include <string.h>
#include "Logger.h"
#include "MQTTMessage.h"
#include "ESPDisplay.h"

class LoRaGateway
{
private:
	static String tag;
	static Logger logger;
	//MQTTMessage mqttmessage;
	ESPDisplay* espDisplay;
public:
	LoRaGateway();
	~LoRaGateway();

	void init(String address, bool serverenabled, ESPDisplay* pDisplay);
	void senderloop();
	String receiverloop();
	//void onReceive(int packetSize);
	void sendLoraMessage(String outgoing);
	//bool sendMQTTMessage(MQTTMessage mqttmessage);
	
	bool gatewayServer = false;
	bool getGatewayServer();

	String targetAddress ="";
	String getTargetAddress();
};

#endif

