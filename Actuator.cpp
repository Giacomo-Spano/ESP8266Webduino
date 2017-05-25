#include "Actuator.h"

Logger Actuator::logger;
String Actuator::tag = "Command";

Actuator::Actuator(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id,pin, enabled, address, name)
{
	//Sensor(pin, enabled, address, name);
	type = "actuator";
}


Actuator::~Actuator()
{
}

void Actuator::checkStatus()
{
}

String Actuator::getJSON()
{
	return "actuator";
}

String Actuator::sendCommand(String json)
{
	return String();
}
