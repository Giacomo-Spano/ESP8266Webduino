#include "HttpHelper.h"


HttpHelper::HttpHelper(/*WiFiClient *client*/)
{
	/*this->pClient = client*/
}


HttpHelper::~HttpHelper()
{


}

bool HttpHelper::post(char* host, int port, char* path, char* param, int len, String *result)
{
	// Use WiFiClient class to create TCP connections
	WiFiClient client;

	String data;
	Serial.println(F("SEND POST"));
	Serial.println(host);
	Serial.println(port);
	Serial.println(path);
	Serial.println(param);

	data = "";
	if (client.connect(host, port)) {
		//Serial.println("connected");

		data += "POST ";
		data += path;
		data += " HTTP/1.1\r\nHost: ";
		data += host;
		data += "\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		data += "Connection: close\r\n";
		data += "Content-Length: ";
		data += len;
		data += "\r\n\r\n";
		data += param;
		data += "\r\n";

		client.print(data);
		//Serial.println(F("post data sent"));
		//Serial.println(data);

		delay(1000);

		// Read all the lines of the reply from server and print them to Serial
		while (client.available()){
			//Serial.println("client.available");
			*result = client.readStringUntil('\r');
			//Serial.print(line);
		}
		//Serial.println();
		//Serial.println("received answer - closing connection");
		//delay(1000);

	}
	else {
		//Serial.println(F("-NON CONNESSO-"));
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	return true;
}
