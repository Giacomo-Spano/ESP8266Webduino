#include "Logger.h"
#include "NextionDisplay.h"

#ifdef ESP8266x

extern SoftwareSerial swSer1(D1, D0, false, 256);
SoftwareSerial* swSer2;// = new SoftwareSerial(*swSer2);;
extern SoftwareSerial nextionSoftwareSerial;// = SoftwareSerial(*swSer2);
//extern SoftwareSerial nextionSoftwareSerial;


//#include "Nextion.h"

//void bOnPopCallback(void *ptr);
// b0;
//NexText t0;

NexButton b0 = NexButton(0, 1, "b0");
NexText t0 = NexText(0, 4, "t0");


NexTouch *nex_listen_list[] = {
	&b0,
	NULL
};

int counter = 0;
void bOnPopCallback(void *ptr) {

	//logger.println(tag, String(F("counter")) + counter);
	String str = "counter ";
	str += counter++;

	t0.setText(str.c_str());
	//digitalWrite(led1, HIGH);
}

Logger NextionDisplay::logger;
String NextionDisplay::tag = "NextionDisplay";

NextionDisplay::NextionDisplay()
{
	
}


NextionDisplay::~NextionDisplay()
{
}

void NextionDisplay::init()
{
	//swSer2 = new SoftwareSerial(D1, D2, false, 256);
	//swSer2->begin(9600);
	//nextionSoftwareSerial = SoftwareSerial(*swSer2); // Cast pointer to object
	
	swSer1.begin(9600);
	
	nexInit();
	
	t0.setText("State: xxxxx");
	b0.attachPop(bOnPopCallback, &b0);
}

void NextionDisplay::loop()
{
	nexLoop(nex_listen_list);
}

#endif