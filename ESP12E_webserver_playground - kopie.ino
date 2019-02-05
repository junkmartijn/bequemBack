#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#include "SettingsManager.h"
#include <Vector.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
const char* configFileName2 = "/config.json";


class MyHandler : public RequestHandler {
	bool canHandle(HTTPMethod method, String uri) {
		return uri.startsWith("/testje"); //uri != null &&
	}

	bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
		Serial.println("testje");
		Serial.println(requestMethod);
		Serial.println(requestUri);
		server.send(200, "text/html", prepareHtmlPage());

	}

	String prepareHtmlPage()
	{
		String htmlPage = "Analog input:  " + String(analogRead(A0));
		return htmlPage;
	}
} myHandler;



bool loadConfig() {
	File configFile = SPIFFS.open(configFileName2, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in loadConfig)");
		return false;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		Serial.println("Failed to parse config file");
		return false;
	}

	const char* serverName = json["serverName"];
	
	// Real world application would store these values in some variables for
	// later use.

	Serial.print("Loaded serverName: ");
	Serial.println(serverName);
	configFile.close();
	return true;
}

bool saveConfig() {
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["serverName"] = "api.example.com";
	JsonArray& tasks = json.createNestedArray("tasks");

	File configFile = SPIFFS.open(configFileName2, "w");
	if (!configFile) {
		Serial.println("Failed to open config file for writing");
		return false;
	}

	json.printTo(configFile);
	configFile.close();
	SettingsManager sm;

	sm.AddTask(Task(1, 10, false));
	sm.AddTask(Task(2, 20, true));
	sm.AddTask(Task(3, 30, false));

	return true;
}

void setup() {
	Serial.begin(115200);         // Start the Serial communication to send messages to the computer
	delay(10);
	Serial.println('\n');

	
	//  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
	//  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

	Serial.println("Connecting ...");
	int i = 0;
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

	SPIFFS.begin();                           // Start the SPI Flash Files System

	if (SPIFFS.exists(configFileName2)) {
		Serial.println("configFile bestaat (in AddTask main)");
	}
	else {
		Serial.println("configFile bestaat niet (in AddTask main), saveConfig aangeroepen");
		if (!saveConfig()) {
			Serial.println("Failed to save config");
		}
		else {
			Serial.println("Config saved");
		}
	}


	if (false) {
		if (!saveConfig()) {
			Serial.println("Failed to save config");
		}
		else {
			Serial.println("Config saved");
		}
	}

	if (false) {
		if (!loadConfig()) {
			Serial.println("Failed to load config");
			Serial.println("Saving default config");
			saveConfig();
			if (!loadConfig()) {
				Serial.println("Failed to load config, even after saving default config!");
			}
			else {
				Serial.println("Config loaded");
			}
		}
		else {
			Serial.println("Config loaded");
		}
	}

	server.onNotFound([]() {                              // If the client requests any URI
	//  server.on("/",[]() {                              // If the client requests any URI
		if (!handleFileRead(server.uri()))                  // send it if it exists
			server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
	});

	server.addHandler(&myHandler);
	server.on("/koekje", testje);


	server.begin();                           // Actually start the server
	Serial.println("HTTP server started");
}

void testje()
{
	Serial.println("methode aangeroepen via server.on");
	SettingsManager sm;
	Task task = Task(3, millis(), false);
	sm.AddTask(task);

	//String x= sm.Config2.serverName + "tasks h" + sm.task000.hour + " m" + sm.task000.min + " s" + sm.task000.state;
	String x = "";
	for (int i = 0; i < sm.Tasks.Size(); i++) {
		x += "<br>";
		x += "h" + (String)sm.Tasks[i].hour;
		x += " m" + (String)sm.Tasks[i].hour;
		x += " state" + (String)sm.Tasks[i].state;
	}


	//Vector  = sm.Config2.tasks;
	   
	server.send(200, "text/html", x);
}

void loop(void) {
	server.handleClient();
}

String getContentType(String filename) { // convert the file extension to the MIME type
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	return "text/plain";
}

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
