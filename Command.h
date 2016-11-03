#pragma once

#include <Arduino.h>
#include "Settings.h"
#include "HttpHelper.h"
#include "OneWireSensors.h"
#include "JSON.h"
#include "Program.h"


class Command
{
public:
	

	Command();
	~Command();

	boolean registerShield(Settings settings, OneWireSensors ows);
	boolean sendActuatorStatus(Settings settings, OneWireSensors ows, Program programSettings);
	boolean sendSensorsStatus(Settings settings, OneWireSensors ows);
};

