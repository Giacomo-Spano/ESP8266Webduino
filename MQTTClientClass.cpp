#include "MQTTClientClass.h"


MQTTClientClass::MQTTClientClass()
{
}


MQTTClientClass::~MQTTClientClass()
{
}
//#ifdef dopo
void MQTTClientClass::init(WiFiClient* espClient)
{
	Serial.println("MQTTClientClass::init");

	client = new PubSubClient(*espClient);	
	
}

PubSubClient & MQTTClientClass::setServer(const char * domain, uint16_t port)
{
	Serial.println("MQTTClientClass::setServer");
	return client->setServer(/*"192.168.1.3"*/domain, port);
}

PubSubClient & MQTTClientClass::setCallback(MQTT_CALLBACK_SIGNATURE)
{
	Serial.println("MQTTClientClass::setCallback");
	return client->setCallback(callback);
}

boolean MQTTClientClass::connected()
{
	//Serial.println("MQTTClientClass::connected");
	return client->connected();
}

boolean MQTTClientClass::connect(const char* id)
{
	return client->connect(id);
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


//#endif