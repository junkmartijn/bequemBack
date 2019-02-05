#include "SettingsManager.h"

const char* configFileName = "/config.json";
const String serverName = "serverName";

Config Config2;
Vector<Task> Tasks;

SettingsManager::SettingsManager()
{
	LoadConfig();
}

SettingsManager::~SettingsManager()
{
}

void SettingsManager::LoadConfig()
{
	Config2.loaded = false;

	SPIFFS.begin();
	File configFile = SPIFFS.open(configFileName, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in GetConfig)");
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		Serial.println("Failed to parse config file");
		return;
	}
	Serial.print("json: ");
	json.prettyPrintTo(Serial);

	const char* val = json[serverName];
	Config2.serverName = val;

	JsonArray& tasks = json["tasks"];
	for (int i = 0; i < tasks.size(); i++) {
		int h = tasks[i]["h"];
		int m = tasks[i]["m"];
		int s = tasks[i]["s"];
		Tasks.PushBack(Task(h, m, (s == 0) ? false : true));
	}

	Serial.println("Aantal taken in Vector: " + (String)Tasks.Size());
	Config2.loaded = true;
}

void SettingsManager::AddTask(Task newTask)
{
	SPIFFS.begin();

	//CP
	if (SPIFFS.exists(configFileName)) {
		Serial.println("configFile bestaat (in AddTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in AddTask)");
	}

	File configFile = SPIFFS.open(configFileName, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in AddTask)");
		//   return "";
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		//  return "";
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	configFile.close();

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());
	//CP

	json["serverName"] = millis();
	JsonArray& tasks = json["tasks"];

	JsonObject& task = jsonBuffer.createObject();
	task["d"] = (int)newTask.dow;
	task["h"] = newTask.hour;
	task["m"] = newTask.min;
	task["s"] = newTask.state ? 1 : 0;
	tasks.add(task);

	File newConfigFile = SPIFFS.open("/config.json", "w");
	if (!newConfigFile) {
		Serial.println("Failed to open config file for writing");
		//return false;
	}

	Serial.println("After set");
	json.prettyPrintTo(Serial);

	json.printTo(newConfigFile);

	newConfigFile.close();

	LoadConfig();
}