#include "HttpHelper.h"
#include "FS.h"

extern Logger logger;

HttpHelper::HttpHelper()
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
	logger.println(tag, F(">>post\n"));
	// Use WiFiClient class to create TCP connections
	WiFiClient client;
		
	String data;
	//logger.print(tag, F("\n\tSEND POST"));
	logger.print(tag, F("\n\n\thost="));
	logger.print(tag, host);
	logger.print(tag, F("\n\tport="));
	logger.print(tag, port);
	logger.print(tag, F("\n\tpath="));
	logger.print(tag, path);
	logger.print(tag, F("\n\tpostparam="));
	logger.print(tag, param);
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
		
		delay(300);

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
		logger.print(tag, "\n\t SERVERNON CONNESSO-\n");
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	logger.println(tag, F("<<post\n"));
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
	logger.print(tag, path);
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
		logger.println(tag, "-NON CONNESSO-");
		*result = "-NON CONNESSO-";
		return false;
	}
	client.stop();
	return true;
}

bool HttpHelper::downloadfile(String filename, String host, int port, String path, String param, String *result)
{
	// Use WiFiClient class to create TCP connections
	WiFiClient client;

	String data;
	logger.print(tag, F("\n\tDOWNLOAD FILE"));
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

		data += "GET ";
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

		// always use this to "mount" the filesystem
		bool result = SPIFFS.begin();
		Serial.println("SPIFFS opened: " + result);
		// this opens the file "f.txt" in read-mode
		if (SPIFFS.remove("/" + filename))
			Serial.println("file f removed");
		else
			Serial.println("file f NOT removed");

		// open the file in write mode
		File f = SPIFFS.open("/" + filename, "w");
		if (!f) {
			Serial.println("file creation failed");
		}
		else
			Serial.println("file f CREATED");
		String line = "";
		// Read all the lines of the reply from server and print them to Serial
		bool header = true;
		Serial.println("start reading ");
		while (client.available()){

			if (header) {
				line = client.readStringUntil('\r\n');
				char c = client.read();
				if (c == '\r') {
					c = client.read();
					if (c == '\n') {
						header = false;
						Serial.println("end of header");
					}
				}
			}
			else {
				line = client.readStringUntil('\n');
				Serial.print(line);
				//if (line.length() > 100) {
				if (f.println(line) <= 0) {
					Serial.println("errore scrittura ");
					//Serial.println("errore scrittura ")
				}
				delay(10);
					//line = "";
				//}
			}
			//delay(100);
		}
		//f.println(line);
		Serial.println();
		Serial.println("received answer - closing connection");
		f.close();
	}
	else {
		logger.print(tag, "-NON CONNESSO-\n");
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
				logger.println(tag, buffer);
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
					logger.print(tag, *param);
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
