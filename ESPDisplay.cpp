// 
// 
// 


#include "ESPDisplay.h"
#include <SPI.h>


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

#define SCK 5 // GPIO5 - SX1278's SCK
#define MISO 19 // GPIO19 - SX1278's MISO
#define MOSI 27 // GPIO27 - SX1278's MOSI
#define SS 18 // GPIO18 - SX1278's CS
#define RST 14 // GPIO14 - SX1278's RESET
#define DI0 26 // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6 // 915E6
void ESPDisplay::init(uint8_t sda, uint8_t scl)
{

#ifdef TTGO
	SPI.begin(SCK, MISO, MOSI, SS);
#endif
	//

	pdisplay = new SSD1306(address, sda, scl);



	if (pdisplay->init()) {
		logger.println(tag, F("Oled diplay init SUCCESSFUL"));
		pdisplay->flipScreenVertically();
		pdisplay->setFont(ArialMT_Plain_10);
		pdisplay->setTextAlignment(TEXT_ALIGN_LEFT);

		pdisplay->drawString(0, 0, F("Restarting...."));
		pdisplay->display();
	}
	else {
		logger.println(tag, F("Oled diplay init FAILED"));
	}
}

void ESPDisplay::update() {
#ifdef TTGO
	pdisplay->clear();
	pdisplay->setTextAlignment(TEXT_ALIGN_LEFT);
	pdisplay->setFont(ArialMT_Plain_10);

	pdisplay->drawString(0, 0, "Sending packet:");
	pdisplay->drawString(90, 0, String(1));
#endif // TTGO

	pdisplay->display();
}

void ESPDisplay::drawString(int x, int y, String txt, const uint8_t* font) {
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

