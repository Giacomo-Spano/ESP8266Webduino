#ifndef _MQTTCLIENTCLASS_h
#define _MQTTCLIENTCLASS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


#include <PubSubClient.h>
#include <ESP8266WiFi.h>

class MQTTClientClass
{
private:
	PubSubClient *client;
public:
	MQTTClientClass();
	~MQTTClientClass();
//#ifdef dopo
	void init(WiFiClient* espClient);

	PubSubClient& setServer(const char * domain, uint16_t port);
	PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);

	boolean connected();
	boolean connect(const char* id);
	int state();
	void disconnect();

	boolean publish(const char* topic, const char* payload);
	boolean subscribe(const char* topic);
	
	boolean loop();
//#endif
};

#endif

