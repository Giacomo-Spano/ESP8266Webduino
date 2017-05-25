#include "CommandResponse.h"
#include "Logger.h"



Logger CommandResponse::logger;
String CommandResponse::tag = "CommandResponse";



CommandResponse::CommandResponse()
{
}

CommandResponse::~CommandResponse()
{
}

String CommandResponse::getJSON() {

	logger.println(tag, ">>getJSON");

	String json = "{";
	json += getJSONFields();
	json += "}";

	logger.println(tag, "<<getJSON " + json);
	return json;
}

String CommandResponse::getJSONFields() {

	logger.println(tag, ">>getJSONFields");

	String json = "";
	json += "\"result\":\"" + result + "\",";
	json += "\"uuid\":\"" + uuid + "\",";
	
	logger.println(tag, ">>getJSONFields" + json);
	return json;
}
