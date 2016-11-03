#include "Util.h"


Util::Util()
{
}


Util::~Util()
{
}

String Util::floatToString(float floatValue) {
	char temp[30];
	sprintf(temp, "%d.%02d", (int)floatValue, (int)(floatValue * 100.0) % 100);
	String str = "";
	str += String(temp);
	return str;
}