// Display.h

#ifndef _ESPDISPLAY_h
#define _ESPDISPLAY_h

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
	//static uint8_t sda;
	//static uint8_t scl;

public:
	ESPDisplay();
	~ESPDisplay();
	void init(uint8_t sda, uint8_t scl);
	void update();
	void clear();
	void drawString(int x, int y, String txt, const uint8_t* font);
	void drawString(int x, int y, String txt);

	

};

#endif

