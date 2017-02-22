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

ESPDisplay::ESPDisplay()
{
	
}

ESPDisplay::~ESPDisplay()
{
}

void ESPDisplay::init()
{
	pdisplay = new SSD1306(0x3c, D7, D6);

	pdisplay->init();
	pdisplay->flipScreenVertically();
	pdisplay->setFont(ArialMT_Plain_10);
	pdisplay->setTextAlignment(TEXT_ALIGN_LEFT);

	pdisplay->drawString(0, 0, "restarting....");
	pdisplay->display();
}

void ESPDisplay::update() {
	pdisplay->display();
}

void ESPDisplay::drawString(int row, int col, String txt,const char* font) {
	pdisplay->setFont(font);
	pdisplay->drawString(row, col, txt);
}
void ESPDisplay::clear() {
	pdisplay->clear();
}

