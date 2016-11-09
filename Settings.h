#pragma once

#include <Arduino.h>

class Settings
{	

public:
	static const int boardnamelen = 30;
	static const int servernamelen = 30;

	Settings();
	~Settings();

	char networkSSID[32];// = "ssid";
	char networkPassword[96];// = "password";
	int localPort = 80;
	char servername[servernamelen];
	int serverPort;
	char boardname[boardnamelen];
	/*uint8_t*/unsigned char MAC_array[6];
	char MAC_char[18];
	byte id = 0; // inizializzato a zero perchè viene impostato dalla chiamata a registershield

	float localTemperature = 0;
	float localAvTemperature = 0;
	float oldLocalAvTemperature = 0;

	String localIP;
};

