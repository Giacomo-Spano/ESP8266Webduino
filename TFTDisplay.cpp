
#ifdef ESP8266

#include "TFTDisplay.h"

// https://learn.adafruit.com/adafruit-gfx-graphics-library?view=all


Logger TFTDisplay::logger;
String TFTDisplay::tag = "TFTDisplay";
Adafruit_ST7735* TFTDisplay::pDisplay;
uint8_t TFTDisplay::_rst = -1;
uint8_t TFTDisplay::_cs = -1;
uint8_t TFTDisplay::_dc = D0; // Funziona con D0, D1, D2, D3, D4, D8
int TFTDisplay::rotation = 3;

uint8_t TFTDisplay::_sclk = D1;   // set these to be whatever pins you like!
uint8_t TFTDisplay::_mosi = D2;   // set these to be whatever pins you like!
bool TFTDisplay::disabled = true;

TFTDisplay::TFTDisplay()
{
}

TFTDisplay::~TFTDisplay()
{
}

void TFTDisplay::init()
{
	if (disabled) return;

	pDisplay = new Adafruit_ST7735(_cs, _dc, _rst);
	pDisplay->initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

	pDisplay->setRotation(rotation);
	clear();
	String str = "prova";
	drawString(0,0,str, 1,ST7735_WHITE);
}

void TFTDisplay::update() {
	//pdisplay->display();
}

void TFTDisplay::drawString(int x, int y, String txt, int size, int color) {
	
	if (disabled) return;

	pDisplay->setCursor(x, y);	
	pDisplay->setTextWrap(true);
	int backgroundcolor = ST7735_BLUE;
	pDisplay->setTextColor(color, backgroundcolor);
	pDisplay->setTextSize(size);
	pDisplay->print(txt);
	//delay(500);
}

void TFTDisplay::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
	int16_t w, int16_t h, uint16_t color) {

	if (disabled) return;

	pDisplay->drawXBitmap(x,y,bitmap,w,h,color);
}

void TFTDisplay::clear() {

	if (disabled) return;

	pDisplay->fillScreen(ST7735_BLUE);
}

#endif