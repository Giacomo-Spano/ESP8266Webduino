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
	//logger.println(tag, ">>writeInt index=" + String(index));
	//logger.print(tag, "\t value=" + String(value));

	int startIndex = index;

	if (index < 0 || (index + 2) > epromSize) return 0;
	
	byte hiByte = highByte(value);
	byte loByte = lowByte(value);
	EEPROM.write(index++, hiByte);
	EEPROM.write(index++, loByte);

	EEPROM.commit();

	//logger.println(tag, "<<writeInt index=" + String(index - startIndex));
	return index - startIndex;
}

int MyEPROMClass::writeJSON(int index, JSONObject *json)
{


	logger.print(tag, "\n\t >>writeJSON index=" + String(index));
	
	int startIndex = index;

	String str = json->toString();

	//logger.print(tag, "\n\t str =" + str/*json->toString()*/);

	if (index < 0 || (index + str.length()/*json->toString().length()*/) > epromSize) return 0;

	index += writeInt(index, str.length()/*json->toString().length()*/);
	
	//logger.print(tag, "\n\t len=" + String(str.length()/*json->toString().length())*/));

	/// qui c'è un bug. Se json è troppo grande va tutto in errore

	//logger.print(tag, "\n\t loop:");
	for (int i = 0; i < str.length()/*json->toString().length()*/; ++i)
	{
		//logger.print(tag, json->toString().charAt(i));

		EEPROM.write(index++, str.charAt(i)/*json->toString().charAt(i)*/);
		
	}

	EEPROM.commit();

	//logger.print(tag, "\n\t <<writeJSON + written " + String(index - startIndex));
	return index - startIndex;

}

int MyEPROMClass::writeString(int index, String *txt)
{
	logger.print(tag, F("1n1t >>writeString index="));
	logger.print(tag, String(index));

	int startIndex = index;

	//logger.print(tag, "\n\t str =" + *txt);

	if (index < 0 || (index + txt->length()) > epromSize) return 0;

	index += writeInt(index, txt->length());

	//logger.print(tag, "\n\t len=" + String(txt->length()));
	for (int i = 0; i < txt->length(); ++i)
	{
		EEPROM.write(index++, txt->charAt(i));
		//if (i > maxjsonLength) break;
	}

	EEPROM.write(index++, 0);

	EEPROM.commit();

	//logger.print(tag, "\n\t <<txt + written " + String(index - startIndex));
	return index - startIndex;

}

int MyEPROMClass::readJSON(int index, JSONObject *json) {

	//logger.print(tag, "\n");
	//logger.println(tag, ">>readJSON index=" + String(index));

	int startIndex = index;

	if (index < 0 || index > epromSize) return -1;

	// read json len
	int len;
	index += readInt(index, &len);
	//logger.print(tag, "\n\t index=" + String(index));
	//logger.print(tag, "\n\t len=" + String(len));

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
	//logger.print(tag, "\n\t str=" + str);

	json->parse(str);
	//logger.println(tag, "<<readJSON json=" + logger.formattedJson(json->toString()));

	return index - startIndex;
}

int MyEPROMClass::readString(int index, String *txt) {

	//logger.print(tag, "\n");
	//logger.println(tag, ">>readString index=" + String(index));

	int startIndex = index;

	if (index < 0 || index > epromSize) return -1;

	// read json len
	int len;
	index += readInt(index, &len);
	//logger.print(tag, "\n\t index=" + String(index));
	//logger.print(tag, "\n\t len=" + String(len));

	if (index + len > epromSize) return -1;

	// read txt
	*txt = "";
	int i = 0;
	while (i < len && i + index < epromSize && i + index > 0)
	{
		char c = char(EEPROM.read(i++ + index));
		*txt += c;
		if (c == 0)
			break;
	}
	*txt += '\0';
	index += i;
	//logger.print(tag, "\n\t txt=" + *txt);

	//logger.println(tag, "<<readString txt=" + *txt);

	return index - startIndex;
}


