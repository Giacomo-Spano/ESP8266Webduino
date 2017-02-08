#include "Actuator.h"

Logger Actuator::logger;
String Actuator::tag = "Command";

Actuator::Actuator()
{
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
