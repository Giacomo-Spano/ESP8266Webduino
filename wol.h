#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>

// wol.h

#ifndef _WOL_h
#define _WOL_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <EthernetUDP.h> // Arduino 1.0 UDP library

class wol
{
private:

	// A UDP instance to let us send and receive packets over UDP
	EthernetUDP Udp;
	char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
	int packetSize; // holds received packet size





public:
	void init();
	void wakeup();
	void inviaMagicPacket();
};

extern wol WOL;

#endif

