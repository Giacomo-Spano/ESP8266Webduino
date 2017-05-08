#include "stdafx.h"
#include "Tokener.h"
//#include "Emulator\ESPEmulator.h"

Logger Tokener::logger;
String Tokener::tag = "Shield";

Tokener::Tokener(String json)
{
	this->json = json;
	current = 0;
}


Tokener::~Tokener()
{
}

// ritorna il prossimo carattere diverso da zero. 
char Tokener::nextClean()
{	
	/*logger.println(tag, "\n");
	logger.println(tag, "nextClean json=-" + json);
	logger.println(tag, "current= " + String(current));
	logger.println(tag, "json.length()= " + String(json.length()));*/

	while (current < json.length()) {

		char c = json[current];		
		if (c == 32 /* space */ || c == 10 /* line feed*/) {
			current++;
			continue;
		}
		current++;
		//logger.println(tag, "<<nextClean c=" + String(c));
		return c;
	}
	//logger.println(tag, "<<nextClean zero char");
	return 0;
}

// ritorna il prossimo carattere diverso da zero senza spostare il cursore. 
char Tokener::nextCleanNoCursor()
{
	int n = current;
	while (n < json.length()) {

		char c = json[n];
		if (c == 32 /* space */ || c == 10 /* line feed*/) {
			n++;
			continue;
		}
		n++;
		return c;
	}
	return 0;
}



// ritorna il prossimo carattere. 
char Tokener::next()
{
	if (current < json.length()) {

		char c = json[current++];
		return c;
	}
	return 0;
}

// ritorna la prossima stringa fino a toChar escluso toChar
String Tokener::nextTo(char toChar)
{
	String token = "";
	while (current < json.length()) {

		char c = json[current++];
		

		if (c == toChar) {
			return token;
		}
		token = token +  c;
		//current++;
	}
	logger.print(tag, "\n\t Token:toChar =" + toChar);

	return "";
}

bool Tokener::nextIsFloat()
{

	if (current < 1) {
		logger.print(tag, "\n\t Tokener::nextIsFloat error");
		return false;
	}


	bool isFloat = false;
	int i = current-1;
	while (i < json.length()) {

		char c = json[i++];

		if (c < '0' && c > '9' //numeric 
			&& c != 45 /* '-' */
			&& c != 46 /* '.' */
			&& c != '}' /* '}' */
			&& c != ',' /* ',' */) {
			logger.print(tag, "\n\t Token:next is float");
			return false;
		}

		if (c == '.')
			isFloat = true;

		if (c == ',' || c == '}') { // ','
			return isFloat;
		}
	}

	logger.print(tag, "\n\t Token:nextIsFloat error");
	return false;
}


float Tokener::nextFloat()
{
	if (current < 1) {
		logger.print(tag, "\n\t Token:nextFloat error");
		return -1.0;
	}

	String token = "";
	token = token + json[current - 1];

	while (current < json.length()) {

		char c = json[current++];

		if (c < '0' && c > '9' //numeric 
			&& c != 45 /* '-' */
			&& c != 46 /* '.' */
			&& c != '}' /* '}' */
			&& c != ',' /* ',' */) {
			logger.print(tag, "\n\t Token:nextFloat number error");
			return -1.0;
		}		

		if (c == ',' || c == '}') { // ','
			float val = (float) token.toFloat();
			current--;
			return val;
		}
		if (c != 32)
			token = token + c;
	}

	logger.print(tag, "\n\t Token:nextIsFloat error");
	return -1.0;
}

int Tokener::nextInteger()
{
	if (current < 1) {
		logger.print(tag, "\n\t Token:nextInt error");
		return -1;
	}


	String token = "";
	token = token + json[current - 1];

	while (current < json.length()) {

		char c = json[current++];

		if (c < '0' && c > '9' //numeric 
			&& c != 45 /* '-' */
			&& c != '}' /* '}' */
			&& c != ',' /* ',' */) {
			logger.print(tag, "\n\t Token:nextInt number error");
			return -1;
		}

		if (c == ',' || c == '}') { // '.'
			int val = token.toInt();
			current--;
			return val;
		}
		if (c != 32)
			token = token + c;
	}

	logger.print(tag, "\n\t Token:nextIsFloat invalid int");
	return -1;
}


bool Tokener::nextBool()
{
	if (current < 1) {
		logger.print(tag, "\n\t Token:nextBool error");
		return false;
	}

	String token = "";
	token = token + json[current - 1];

	while (current < json.length()) {

		char c = json[current++];

		if (c == ',' || c == '}') { // ','
			bool val;
			if (token == "true")
				val = true;
			else if (token == "false")
				val = false;
			else {
				logger.print(tag, "\n\t Token:nextBool error");
				return false;
			}

			current--;
			return val;
		}
		if (c != 32)
			token = token + c;
	}

	logger.print(tag, "\n\t Token:nextBool error");
	return false;
}

String Tokener::nextArray()
{
	//logger.print(tag, "\n");
	//logger.println(tag, ">>nextArray current="+String(current));
	
	if (current < 1)
	{
		logger.print(tag, "\n\t Token:nextArray error");
		return "";
	}

	String token = "";
	//token = token + json[current - 1];
	current--;

	int count = 0;
	while (current < json.length()) {

		//logger.print(tag, "\n\t token="+token);
		//logger.print(tag, " current=" + String(current));

		char c = json[current++];
		//logger.print(tag, "\n\t c=" + String(c));

		if (c == '[') {
			count++;
		}

		if (c == ']') {
			count--;
			if (count == 0) {
				token = token + c;
				//logger.print(tag, "\n\t END token=" + token);
				return token;
			}
		}
		
		token = token + c;
	}

	logger.print(tag, "\n\t Token:nextArray error");
	return "";
}


String Tokener::nextJson()
{
	if (current < 1) {
		logger.print(tag, "\n\t Token:nextJson error");
		return "";
	}

	String token = "";
	current--;

	int count = 0;
	while (current < json.length()) {

		char c = json[current++];

		if (c == '{') {
			count++;
		}

		if (c == '}') {
			count--;
			if (count == 0) {
				token = token + c;
				return token;
			}
		}

		token = token + c;
	}

	logger.print(tag, "\n\t Token:nextJson error");
	return "";
}


