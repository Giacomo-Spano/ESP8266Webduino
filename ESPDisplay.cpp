// 
// 
// 


#include "ESPDisplay.h"


// Initialize the OLED display using Wire library
// D7 -> SDA
// D6 -> SCL
//SSD1306  display(0x3c, D7, D6);

Logger ESPDisplay::logger;
String ESPDisplay::tag = "ESPDisplay";
//SSD1306 ESPDisplay::display(0x3c, D7, D6);
SSD1306* ESPDisplay::pdisplay;
uint8_t ESPDisplay::address = 0x3c;
#ifdef ESP8266
//uint8_t ESPDisplay::sda = D4;
//uint8_t ESPDisplay::scl = D3;
#else
//uint8_t ESPDisplay::sda = 0;
//uint8_t ESPDisplay::scl = 0;
#endif

ESPDisplay::ESPDisplay()
{
	
}

ESPDisplay::~ESPDisplay()
{
}

void ESPDisplay::init(uint8_t sda, uint8_t scl)
{
	//pdisplay = new SSD1306(0x3c, D7, D6);
	pdisplay = new SSD1306(address, sda, scl);

	pdisplay->init();
	pdisplay->flipScreenVertically();
	pdisplay->setFont(ArialMT_Plain_10);
	pdisplay->setTextAlignment(TEXT_ALIGN_LEFT);

	pdisplay->drawString(0, 0, F("Restarting...."));
	pdisplay->display();
}

void ESPDisplay::update() {
	pdisplay->display();
}

void ESPDisplay::drawString(int x, int y, String txt,const char* font) {
	pdisplay->setFont(font);
	pdisplay->drawString(x, y, txt);
}

void ESPDisplay::drawString(int x, int y, String txt) {
	//pdisplay->setFont(font);
	pdisplay->drawString(x, y, txt);
}

void ESPDisplay::clear() {
	pdisplay->clear();
}

