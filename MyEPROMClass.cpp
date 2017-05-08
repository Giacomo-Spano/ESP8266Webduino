#include "MyEPROMClass.h"
#include "EEPROM.h"

Logger MyEPROMClass::logger;
String MyEPROMClass::tag = "Sensor";
bool MyEPROMClass::epromInitialized = true;


MyEPROMClass::MyEPROMClass()
{
	if (!epromInitialized)
		init();
	
}

MyEPROMClass::~MyEPROMClass()
{
}

void MyEPROMClass::init() 
{
	EEPROM.begin(epromSize);
	epromInitialized = true;
}

int MyEPROMClass::readInt(int index, int *value)
{
	int startIndex = index;
	byte hiByte = EEPROM.read(index++);
	byte loyte = EEPROM.read(index++);
	*value = word(hiByte, loyte);

	return index - startIndex;
}

int MyEPROMClass::writeInt(int index, int value)
{
	logger.println(tag, ">>writeInt index=" + String(index));
	logger.print(tag, "\t value=" + String(value));

	int startIndex = index;

	if (index < 0 || (index + 2) > epromSize) return 0;
	
	byte hiByte = highByte(value);
	byte loByte = lowByte(value);
	EEPROM.write(index++, hiByte);
	EEPROM.write(index++, loByte);

	EEPROM.commit();

	logger.println(tag, "<<writeInt index=" + String(index - startIndex));
	return index - startIndex;
}

int MyEPROMClass::writeJSON(int index, JSONObject *json)
{
	logger.print(tag, "\n\t >>writeJSON index=" + String(index));
	
	int startIndex = index;

	logger.print(tag, "\n\t str =" + json->toString());

	if (index < 0 || (index + json->toString().length()) > epromSize) return 0;

	index += writeInt(index, json->toString().length());
	
	logger.print(tag, "\n\t len=" + String(json->toString().length()));
	for (int i = 0; i < json->toString().length(); ++i)
	{
		EEPROM.write(index++, json->toString().charAt(i));
		//if (i > maxjsonLength) break;
	}

	EEPROM.commit();

	logger.print(tag, "\n\t <<writeJSON + written " + String(index - startIndex));
	return index - startIndex;

}

int MyEPROMClass::readJSON(int index, JSONObject *json) {

	logger.print(tag, "\n");
	logger.println(tag, ">>readJSON index=" + String(index));

	int startIndex = index;

	if (index < 0 || index > epromSize) return -1;

	// read json len
	int len;
	index += readInt(index, &len);
	logger.print(tag, "\n\t index=" + String(index));
	logger.print(tag, "\n\t len=" + String(len));

	if (index + len > epromSize) return -1;

	// read json
	String str = "";
	int i = 0;
	while (i < len && i + index < epromSize && i + index > 0)
	{
		char c = char(EEPROM.read(i++ + index));
		str += c;
		if (c == 0)
			break;
	}
	str += '\0';
	index += i;
	logger.print(tag, "\n\t str=" + str);

	json->parse(str);
	logger.println(tag, "<<readJSON json=" + logger.formattedJson(json->toString()));

	return index - startIndex;
}


