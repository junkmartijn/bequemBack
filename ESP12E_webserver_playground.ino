#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#include "SettingsManager.h"
#include <Vector.h>
#include "config.h"

SettingsManager settingsManager;
ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

#pragma region setup
void setup() {
	SPIFFS.begin();

	Serial.begin(115200);         // Start the Serial communication to send messages to the computer
	delay(10);
	wifiMulti.addAP(wifiSsid, wifiPass);

	Serial.println("Connecting ...");
	while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
		delay(250);
		Serial.print('.');
	}
	Serial.println('\n');
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());              // Tell us what network we're connected to
	Serial.print("IP address:\t");
	Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

	if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
		Serial.println("mDNS responder started");
	}
	else {
		Serial.println("Error setting up MDNS responder!");
	}

	server.begin();
	server.on("/api/tasks", HTTP_GET, WebApiTasksGet);
	server.on("/api/tasks", HTTP_DELETE, WebApiTasksDelete);
	server.on("/api/task", HTTP_POST, WebApiTaskPost);
	server.on("/api/task", HTTP_DELETE, WebApiTaskDelete);
	//server.on("/api/", WebApiNotFound);
	server.onNotFound([]() {
		if (server.method() == HTTP_OPTIONS)
		{
			SendHeaders();
			server.send(204);
		}
		else
		{
			Serial.println("NotFound, uri: " + server.uri() + " method: " + server.method());
			if (!handleFileRead(server.uri()))                  // send it if it exists
			{
				server.send(404, "text/plain", "404: Not Found");// otherwise, respond with a 404 (Not Found) error
			}
		}

	});
	Serial.println("HTTP server started");
}
#pragma endregion setup

void loop(void) {
	server.handleClient();
}

#pragma region fileserver

bool handleFileRead(String path) { // send the right file to the client (if it exists)

	Serial.println("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
	String contentType = getContentType(path);            // Get the MIME type
	if (SPIFFS.exists(path)) {                            // If the file exists
		File file = SPIFFS.open(path, "r");                 // Open it
		size_t sent = server.streamFile(file, contentType); // And send it to the client
		file.close();                                       // Then close the file again
		return true;
	}
	Serial.println("\tFile Not Found");

	return false;                                         // If the file doesn't exist, return false
}

String getContentType(String filename) { // convert the file extension to the MIME type
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	return "text/plain";
}

#pragma endregion fileserver

#pragma region webapi methods
void WebApiTasksGet()
{
	SendHeaders();
	server.send(200, "text/json", settingsManager.TasksJson);
}

void SendHeaders() {
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.sendHeader("Access-Control-Max-Age", "10000");
	server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,DELETE,OPTIONS");
	server.sendHeader("Access-Control-Allow-Headers", "*");
}

void WebApiTaskPost()
{
	SendHeaders();

	if (server.hasArg("plain") == false) {
		server.send(404, "text/plain", "Body not received");
		return;
	}
	auto taskString = server.arg("plain");

	Serial.println("AddTask1: " + taskString);

	DynamicJsonBuffer jsonBuffer;
	JsonObject& taskJson = jsonBuffer.parseObject(taskString);

	if (!taskJson.containsKey("d")
		|| !taskJson.containsKey("h")
		|| !taskJson.containsKey("m")
		|| !taskJson.containsKey("s")
		) {
		server.send(404, "text/plain", "Invalid body:" + taskString);
		return;
	}

	int d = taskJson["d"];// 0 if not provided
	int h = taskJson["h"];
	int m = taskJson["m"];
	int s = taskJson["s"];

	if (d < 1 || d>8 //8 is all days
		|| h < 0 || h>23
		|| m < 0 || m>59
		|| s < 0 || s>1) {
		server.send(404, "text/plain", "Invalid body");
		return;
	}

	Serial.println("AddTask2: " + taskString);

	auto task = Task(d, h, m, (bool)s);

	settingsManager.AddTask(task);

	String message = "Body received:";
	message += " dow:" + (String)d;
	message += " hour:" + (String)h;
	message += " min:" + (String)m;
	message += " state:" + (String)s;
	//message += "\n";

	Serial.println("AddTask: " + message);

	server.send(200, "text/plain", message);
}

void WebApiTaskDelete() {
	SendHeaders();

	String messageArgs = "";
	for (int i = 0; i < server.args(); i++) {

		messageArgs += "Arg no" + (String)i + " – > ";
		messageArgs += server.argName(i) + ": ";
		messageArgs += server.arg(i) + "\n";

	}
	Serial.print("Args: ");
	Serial.println(messageArgs);


	if (!server.hasArg("d")
		|| !server.hasArg("h")
		|| !server.hasArg("m")
		|| !server.hasArg("s")
		) {
		server.send(404, "text/plain", "Invalid request");
		return;
	}


	int d = server.arg("d").toInt();
	int h = server.arg("h").toInt();
	int m = server.arg("m").toInt();
	int s = server.arg("s").toInt();

	Serial.println("d" + (String)d + " h" + (String)h + " m" + (String)m + " s" + (String)s);

	if (d < 0 || d>7
		|| h < 0 || h>23
		|| m < 0 || m>59
		|| s < 0 || s>1) {
		server.send(404, "text/plain", "Invalid request");
		return;
	}

	auto task = Task(d, h, m, (bool)s);

	settingsManager.RemoveTask(task);

	String message = "Body received:";
	message += " dow:" + (String)d;
	message += " hour:" + (String)h;
	message += " min:" + (String)m;
	message += " state:" + (String)s;
	//message += "\n";

	Serial.println("DeleteTask: " + message);

	server.send(200, "text/plain", message);
}

void WebApiTasksDelete() {

	settingsManager.RemoveTasks();
	server.send(200, "text/plain", "Success");
}

void WebApiNotFound() {
	server.send(404, "text/plain", "No endpoint listening at " + server.uri() + " with method " + server.method());
}

#pragma endregion webapi methods