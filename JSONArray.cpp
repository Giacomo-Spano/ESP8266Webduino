// 
// 
// 

#include "JSONArray.h"

Logger JSONArray::logger;
String JSONArray::tag = "JSONArray";

JSONArray::JSONArray(String json)
{
	jsonArrayString = json;
	start = 0;
	end = 0;
}

JSONArray::~JSONArray()
{
	start = 0;
	end = 0;
}

String JSONArray::getFirst()
{
	start = 0;
	end = 0;
	return getNext();
}

String JSONArray::getNext()
{
	//logger.print(tag, "\n\t getNext()");

	start = jsonArrayString.indexOf('{',end);
	if (start == -1)
		return "";
	int i = start+1;
	int count = 1;
	while (i < jsonArrayString.length()) {

		if (jsonArrayString.charAt(i) == 123) {
			count++;
		}

		if (jsonArrayString.charAt(i) == 125) {
			end = i;			
			count--;
			if (count == 0) {
				String jsonString = jsonArrayString.substring(start, end + 1);
				return jsonString;
			}
			
		}
		i++;
	}

	return "";
}
