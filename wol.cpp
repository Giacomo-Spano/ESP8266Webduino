// 
// 
// 

#include "wol.h"


//char answerPacketBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,

//Indirizzo broadcast della rete locale
static byte broadCastIp[] = { 192, 168, 1, 255 };
//Mac address della scheda lan del pc da avviare
static byte remote_MAC_ADD[] = { 0x00, 0x22, 0x4D, 0x7C, 0x36, 0xF2 }; ///00:22:4D:7C:36:F2  INDIRIZZO IP DEL PC DA SVEGLIARE
//Porta UDP WOL
static int wolPort = 9;
static unsigned int localPort = 7777; // local port to listen on

void wol::init()
{
	Serial.print(F("wol::print"));

}

void wol::wakeup() {

	Udp.begin(localPort);


	Serial.println("wakeup");
	inviaMagicPacket();
	Serial.println("wakeup packet sent");

	Udp.stop();
}

void wol::inviaMagicPacket()
{
	Serial.println("inviaMagicPacket");

	//definisco un array da 102 byte
	byte magicPacket[102];
	//variabili per i cicli
	int Ciclo = 0, CicloMacAdd = 0, IndiceArray = 0;

	for( Ciclo = 0; Ciclo < 6; Ciclo++)
	{
		//i primi 6 byte dell'array sono settati al valore 0xFF
		magicPacket[IndiceArray] = 0xFF;
		//incremento l'indice dell'array
		IndiceArray++;
	}

	//eseguo 16 cicli per memorizzare il mac address del pc
	//da avviare
	for( Ciclo = 0; Ciclo < 16; Ciclo++ )
	{
		//eseguo un ciclo per memorizzare i 6 byte del
		//mac address
		for( CicloMacAdd = 0; CicloMacAdd < 6; CicloMacAdd++)
		{
			magicPacket[IndiceArray] = remote_MAC_ADD[CicloMacAdd];
			//incremento l'indice dell'array
			IndiceArray++;
		}
	}

	//spedisco il magic packet in brodcast sulla porta prescelta
	Udp.beginPacket(broadCastIp, wolPort);
	Udp.write(magicPacket, sizeof magicPacket);
	Udp.endPacket();

}


wol WOL;

