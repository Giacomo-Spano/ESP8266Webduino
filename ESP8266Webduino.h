// ESP8266Webduino.h

#ifndef _ESP8266WEBDUINO_h
#define _ESP8266WEBDUINO_h

#define ESP8266

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <OneWire.h>
#include <DallasTemperature.h>
/*
class OneWire;
extern OneWire* oneWirePtr;
class DallasTemperature;
extern DallasTemperature* pDallasSensors;
*/


#endif

