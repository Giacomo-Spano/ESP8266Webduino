#ifndef _MQTTCLIENTCLASS_h
#define _MQTTCLIENTCLASS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Logger.h"
#include <PubSubClient.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include "WiFi.h"
#endif


class MQTTClientClass
{
private:
	static String tag;
	static Logger logger;

	PubSubClient *client;
public:
	MQTTClientClass();
	~MQTTClientClass();
//#ifdef dopo
	void init(WiFiClient* espClient);

	PubSubClient& setServer(String domain, uint16_t port);
	PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);

	boolean connected();
	boolean connect(String id, String user, String password);
	int state();
	void disconnect();

	boolean publish(const char* topic, const char* payload);
	boolean subscribe(const char* topic);
	
	boolean loop();
//#endif
};

#endif

