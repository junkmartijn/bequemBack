#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#include "SettingsManager.h"
#include "config.h"

SettingsManager settings_manager;
ESP8266WiFiMulti wifi_multi;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

const int output_pin = 2;

#pragma region setup
void setup() {
	SPIFFS.begin();

	pinMode(output_pin, OUTPUT);
	digitalWrite(output_pin, HIGH);

	setTime(16, 33, 0, 23, 2, 2019);


	Serial.begin(115200);         // Start the Serial communication to send messages to the computer
	delay(10);
	wifi_multi.addAP(wifiSsid, wifiPass);

	Serial.println("Connecting ...");
	while (wifi_multi.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
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
	server.on("/api/alarms", HTTP_POST, WebApiAlarms);
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

	settings_manager.MakeAlarms(ActionOn, ActionOff);
}
#pragma endregion setup


void loop(void) {
	server.handleClient();
	Alarm.delay(1);
}

#pragma region actions


void ActionOn() {
	Serial.println("ActionOn");
	Serial.println(String(hour()) + ":" + String(minute()));
	const bool force_constant = LOW;
	if (force_constant == LOW) {
		digitalWrite(output_pin, LOW);
	}
}

void ActionOff() {
	Serial.println("ActionOff");
	Serial.println(String(hour())+":"+String(minute()));
	const bool force_constant = LOW;
	if (force_constant == LOW) {
		digitalWrite(output_pin, HIGH);
	}
}
#pragma endregion actions




#pragma region fileserver

bool handleFileRead(String path) { // send the right file to the client (if it exists)

	Serial.println("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
	const String content_type = getContentType(path);            // Get the MIME type
	if (SPIFFS.exists(path)) {                            // If the file exists
		auto file = SPIFFS.open(path, "r");                 // Open it
		size_t sent = server.streamFile(file, content_type); // And send it to the client
		file.close();                                       // Then close the file again
		return true;
	}
	Serial.println("\tFile Not Found");

	return false;                                         // If the file doesn't exist, return false
}

String getContentType(const String& filename) { // convert the file extension to the MIME type
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
	server.send(200, "text/json", settings_manager.TasksJson);
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
	auto task_string = server.arg("plain");

	Serial.println("AddTask1: " + task_string);

	DynamicJsonBuffer jsonBuffer;
	auto& task_json = jsonBuffer.parseObject(task_string);

	if (!task_json.containsKey("d")
		|| !task_json.containsKey("h")
		|| !task_json.containsKey("m")
		|| !task_json.containsKey("s")
		) {
		server.send(404, "text/plain", "Invalid body:" + task_string);
		return;
	}

	const int d = task_json["d"];// 0 if not provided
	const int h = task_json["h"];
	const int m = task_json["m"];
	const int s = task_json["s"];

	if (d < 1 || d>8 //8 is all days
		|| h < 0 || h>23
		|| m < 0 || m>59
		|| s < 0 || s>1) {
		server.send(404, "text/plain", "Invalid body");
		return;
	}

	Serial.println("AddTask2: " + task_string);

	const auto task = Task(d, h, m, bool(s));

	const auto result = settings_manager.AddTask(task);

	String message = "Body received:";
	message += " dow:" + String(d);
	message += " hour:" + String(h);
	message += " min:" + String(m);
	message += " state:" + String(s);
	
	Serial.println("AddTask: " + message);

	server.send(200, "text/plain", result);
}

void WebApiTaskDelete() {
	SendHeaders();

	String message_args = "";
	for (auto i = 0; i < server.args(); i++) {

		message_args += "Arg no" + String(i) + " – > ";
		message_args += server.argName(i) + ": ";
		message_args += server.arg(i) + "\n";

	}
	Serial.print("Args: ");
	Serial.println(message_args);


	if (!server.hasArg("d")
		|| !server.hasArg("h")
		|| !server.hasArg("m")
		|| !server.hasArg("s")
		) {
		server.send(404, "text/plain", "Invalid request");
		return;
	}


	const int d = server.arg("d").toInt();
	const int h = server.arg("h").toInt();
	const int m = server.arg("m").toInt();
	const int s = server.arg("s").toInt();

	Serial.println("d" + String(d) + " h" + String(h) + " m" + String(m) + " s" + String(s));

	if (d < 0 || d>8
		|| h < 0 || h>23
		|| m < 0 || m>59
		|| s < 0 || s>1) {
		server.send(404, "text/plain", "Invalid request");
		return;
	}

	const auto task = Task(d, h, m, bool(s));

	settings_manager.RemoveTask(task);

	String message = "Body received:";
	message += " dow:" + String(d);
	message += " hour:" + String(h);
	message += " min:" + String(m);
	message += " state:" + String(s);

	Serial.println("DeleteTask: " + message);

	//server.send(200, "text/plain", message);
	server.send(200, "text/plain", "Success");
}

void WebApiTasksDelete() {

	settings_manager.RemoveTasks();
	server.send(200, "text/plain", "Success");
}

void WebApiAlarms() {
	const auto result = settings_manager.MakeAlarms(ActionOn, ActionOff);
	server.send(200, "text/plain", result);
}

void WebApiNotFound() {
	server.send(404, "text/plain", "No endpoint listening at " + server.uri() + " with method " + server.method());
}

#pragma endregion webapi methods