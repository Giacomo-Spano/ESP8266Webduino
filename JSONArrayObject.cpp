
#include "stdafx.h"

#include "JSONArrayObject.h"
#include "Tokener.h"
#include "JSONObject.h"

Logger JSONArrayObject::logger;
String JSONArrayObject::tag = "JSONArrayObject";

JSONArrayObject::JSONArrayObject(String str)
{
	parse(str);
}

JSONArrayObject::~JSONArrayObject()
{
	jsonArray.clearAll();
}

void JSONArrayObject::parse(String jsonstr)
{
	Tokener tokener(jsonstr);
	char c = tokener.nextClean();

	if (c == '[') {

		while (c!=0) {

			c = tokener.nextClean();

			if (c == '{') { // '"'
				String elem = tokener.nextJson();
				JSONObject* jsonElem = new JSONObject(elem);
				jsonArray.add(jsonElem);
			}
			else {
				logger.print(tag, "\n\t JSONArrayObject::parse invalid format =" + jsonstr);
			}

			c = tokener.nextClean();
			if (c == 44) { // ','
				continue;
			}
			else if (c == ']') { // '}'
				return;
			}

		}

		logger.print(tag, "\n\t JSONArrayObject::parse tinvelid format =" + jsonstr);

	}
}

