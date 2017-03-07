// Display.h

#ifndef _ESPDISPLAY_h
#define _ESPDDISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"
// Include the correct display library
// For a connection via I2C using Wire include

//#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
//#include "ESP8266Webduino.h"
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

class ESPDisplay
{
private:
	static String tag;
	static Logger logger;
	//static SSD1306  display;
	static SSD1306* pdisplay;

	static uint8_t address;
	static uint8_t sda;
	static uint8_t scl;

public:
	ESPDisplay();
	~ESPDisplay();
	void init();
	void update();
	void clear();
	void drawString(int rox, int col, String txt, const char* font);

};

#endif

