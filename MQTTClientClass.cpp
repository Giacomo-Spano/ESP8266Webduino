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

//static const char serverMQTT[] = "79.24.3.210";
static const char serverMQTT[] = "giacomohome.ddns.net";
//static const char serverMQTT[] = "192.168.1.3";

PubSubClient & MQTTClientClass::setServer(const char * domain, uint16_t port)
{
	Serial.println("MQTTClientClass::setServer");
	return client->setServer(serverMQTT, port);
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


//#endif