#include "POSTData.h"

Logger POSTData::logger;
String POSTData::tag = "POSTData";

POSTData::POSTData(String json)
{
	this->dataString = json;
	String tag = "POSTData";
}

POSTData::~POSTData()
{
}

bool POSTData::has(String key) {

	String data = dataString;
	key = key + "=";

	int index = data.indexOf(key);
	if (index < 0) {
		//logger.print(tag, "\n\tkey " + key + "not found");
		return false;
	}

	/*data = data.substring(index + key.length());
	index = data.indexOf("=");
	if (index < 0) {
		logger.print(tag, "\n\t: char not found");
		return false;
	}*/

	//logger.print(tag, "\n\tkey " + key + "found");
	return true;
}

String POSTData::getDataString() {
	return dataString;
}

String POSTData::getString(String key) {

	//logger.print(tag, "\n\t getstring " + key);
	String data = dataString;
	key += "=";

	int index = data.indexOf(key);
	//logger.print(tag, "\n\t index=" + String(index));
	if (index < 0) return "";

	data = data.substring(index + key.length());
	//logger.print(tag, "\n\t data=" + data);

	index = data.indexOf("&");
	//logger.print(tag, "\n\t index=" + String(index));
	if (index < 0) {
		//logger.print(tag, "\n\t data=" + data);
		return data;
	}
	else
		data = data.substring(index + 1);

	//logger.print(tag, "\n\t data=" + data);
	return data;
}



