#pragma once
#include <Arduino.h>
#include "Logger.h"

class MQTTMessage
{
public:
	MQTTMessage();
	~MQTTMessage();

	String topic;
	String message;
};

