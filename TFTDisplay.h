// TFTDisplay.h

#ifndef _TFTDISPLAY_h
#define _TFTDISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

class TFTDisplay
{
private:
	static String tag;
	static Logger logger;
	static bool disabled;
	
	//static SSD1306* pdisplay;
	static Adafruit_ST7735* pDisplay;
	static uint8_t _rst;
	static uint8_t _cs;
	static uint8_t _dc;
	static uint8_t _sclk;
	static uint8_t _mosi;
	static int rotation;

	//void drawtext(char *text, uint16_t color);
	

public:
	TFTDisplay();
	~TFTDisplay();
	void init();
	void update();
	void clear();
	//void drawString(int rox, int col, String txt, const char* font);
	void drawString(int x, int y, String txt, int size, int color);
	void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
		int16_t w, int16_t h, uint16_t color);

};

#endif

