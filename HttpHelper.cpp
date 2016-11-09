#include "HttpHelper.h"

extern Logger logger;

HttpHelper::HttpHelper(/*WiFiClient *client*/)
{
	tag = "HttpHelper";
}


HttpHelper::~HttpHelper()
{


}

bool HttpHelper::post(char* host, int port, char* path, char* param, int len, String *result)
{
	post(String(host), port, path, param, len, result);
}

bool HttpHelper::post(String host, int port, String path, String param, String *result)
{
	// Use WiFiClient class to create TCP connections
	WiFiClient client;

	String data;
	logger.print(tag, F("SEND POST"));
	logger.print(tag, F(" host="));
	logger.print(tag, host);
	logger.print(tag, F(",port="));
	logger.print(tag, port);
	logger.print(tag, F(",path="));
	logger.println(tag, path);
	/*logger.print(tag, F("\n\tpostparam="));
	logger.print(tag, param);*/
	/*logger.print(tag, F("\n\tlen="));
	logger.print(tag, len);*/

	char buffer[100];
	host.toCharArray(buffer, sizeof(buffer));

	data = "";
	if (client.connect(buffer, port)) {
		//Serial.println("connected");

		data += "POST ";
		data += path;
		data += " HTTP/1.1\r\nHost: ";
		data += host;
		data += "\r\nContent-Type: application/x-www-form-urlencoded\r\n";
		data += "Connection: close\r\n";
		data += "Content-Length: ";
		data += param.length();
		data += "\r\n\r\n";
		data += param;
		data += "\r\n";

		client.print(data);
		//Serial.println(F("post data sent"));
		//Serial.println(data);

		delay(3000);

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
		logger.println(tag, "-NON CONNESSO-");
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	return true;
}

bool HttpHelper::post(String host, int port, char* path, char* param, int len, String *result)
{
	// Use WiFiClient class to create TCP connections
	WiFiClient client;

	String data;
	logger.print(tag, F("SEND POST"));
	logger.print(tag, F(" host="));
	logger.print(tag, host);
	logger.print(tag, F(",port="));
	logger.print(tag, port);
	logger.print(tag, F(",path="));
	logger.println(tag, path);
	/*logger.print(tag, F("\n\tpostparam="));
	logger.print(tag, param);*/
	/*logger.print(tag, F("\n\tlen="));
	logger.print(tag, len);*/

	char buffer[100];
	host.toCharArray(buffer, sizeof(buffer));

	data = "";
	if (client.connect(buffer, port)) {
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

		delay(3000);

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
		logger.println(tag,"-NON CONNESSO-");
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	return true;
}

boolean HttpHelper::getNextPage(WiFiClient* pClient, WiFiServer* server, String* page, String* param)
{
	//logger.print(tag, F("called getNextPage: "));

	char buffer[HttpHelper::MAX_PAGE_NAME_LEN + 1]; // additional character for terminating null
	char parambuffer[HttpHelper::MAX_PARAM_LEN];

		int type = 0;
		while (pClient->connected()) {

			if (pClient->available()) {
				// GET, POST, or HEAD
				memset(buffer, 0, sizeof(buffer)); // clear the buffer
				memset(parambuffer, 0, sizeof(parambuffer));
				if (pClient->readBytesUntil('/', buffer, MAX_PAGE_NAME_LEN)){
					logger.println(tag,buffer);
					if (strcmp(buffer, "GET ") == 0)
						type = 1;
					else if (strcmp(buffer, "POST ") == 0)
						type = 2;
					// look for the page name
					memset(buffer, 0, sizeof(buffer)); // clear the buffer
					int l;

					if (l = pClient->readBytesUntil(' ', buffer, MAX_PAGE_NAME_LEN))
					{
						//logger.println(tag, l);
						//logger.println(tag, buffer);
						l = findIndex(buffer, "?");
						int i = 0;
						if (l != -1) {
							while ((l + i) < MAX_PAGE_NAME_LEN && i < MAX_PARAM_LEN) {
								parambuffer[i] = buffer[l + i];
								i++;
							}
							buffer[l] = '\0';
						}
						else {
							;
						}
						//logger.println(tag, l);
						//logger.println(tag, buffer);
						//logger.println(tag, parambuffer);
						//logger.println(tag, "-");
						*page = String(buffer);
						*param = String(parambuffer);

						logger.print(tag, F("called getNextPage: "));
						logger.print(tag, " page=");
						logger.print(tag, *page);
						logger.print(tag, ",param=");
						logger.println(tag, *param);
					}
				}
				return true;
			}
		}
		// give the web browser time to receive the data
		//delay(20);
		//client.stop();
	return false;
}

int HttpHelper::findIndex(const char* data, const char* target) {

	boolean found = false;
	int i = 0;
	while (data[i] != '\0') {
		i++;
	}
	i = 0;
	int k = 0;
	while (data[i] != '\0') {

		if (data[i] == target[0]) {
			found = true;
			k = 0;
			while (target[k] != '\0') {

				if (data[i + k] == '\0')
					return -1;
				if (data[i + k] != target[k]) {
					found = false;
					break;
				}
				k++;
			}
			if (found == true)
				return i;
		}
		i++;
	}
	return -1;
}
