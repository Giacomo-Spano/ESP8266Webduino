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
	logger.println(tag, "MQTTClientClass::init");

	client = new PubSubClient(*espClient);		
}

PubSubClient & MQTTClientClass::setServer(String domain, uint16_t port)
{
	logger.println(tag, "MQTTClientClass::setServer");

	IPAddress ipaddr;
	WiFi.hostByName(domain.c_str(), ipaddr);
	Serial.print(domain);
	Serial.print(":");
	Serial.println(port,DEC);

	return client->setServer(ipaddr, port);
}

PubSubClient & MQTTClientClass::setCallback(MQTT_CALLBACK_SIGNATURE)
{
	logger.println(tag, "MQTTClientClass::setCallback");
	return client->setCallback(callback);
}

boolean MQTTClientClass::connected()
{
	return client->connected();
}

boolean MQTTClientClass::connect(String clientid, String user, String password)
{
	//return client->connect(clientid.c_str());
	logger.print(tag, "\n\t connect clientid:" + clientid + " user:" + user + " password:" + password);
	return client->connect(clientid.c_str(), user.c_str(), password.c_str());
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
	bool res = client->publish(topic, payload);
	if (!res)
		logger.print(tag, "\n\t PUBLISH FAILED");
	return res;
}

boolean MQTTClientClass::subscribe(const char* topic)
{
	return client->subscribe(topic);
}

boolean MQTTClientClass::loop()
{
	return client->loop();
}
