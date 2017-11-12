#include "MQTTClientClass.h"

Logger MQTTClientClass::logger;
String MQTTClientClass::tag = "Command";

MQTTClientClass::MQTTClientClass()
{
}


MQTTClientClass::~MQTTClientClass()
{
}

void MQTTClientClass::init(WiFiClient* espClient)
{
	Serial.println("MQTTClientClass::init");

	client = new PubSubClient(*espClient);		
}

PubSubClient & MQTTClientClass::setServer(String domain, uint16_t port)
{
	Serial.println("MQTTClientClass::setServer");

	IPAddress ipaddr;
	WiFi.hostByName(domain.c_str(), ipaddr);
	Serial.print(domain);
	Serial.print(": ");
	Serial.println(port,DEC);

	return client->setServer(ipaddr, port);
}

PubSubClient & MQTTClientClass::setCallback(MQTT_CALLBACK_SIGNATURE)
{
	Serial.println("MQTTClientClass::setCallback");
	return client->setCallback(callback);
}

boolean MQTTClientClass::connected()
{
	return client->connected();
}

boolean MQTTClientClass::connect(String clientid)
{
	return client->connect(clientid.c_str());
}

int MQTTClientClass::state()
{
	return client->state();
}

void MQTTClientClass::disconnect()
{
	return client->disconnect();
}

boolean MQTTClientClass::publish(const char* topic, const char* payload)
{
	return client->publish(topic, payload);
}

boolean MQTTClientClass::subscribe(const char* topic)
{
	return client->subscribe(topic);
}

boolean MQTTClientClass::loop()
{
	return client->loop();
}
