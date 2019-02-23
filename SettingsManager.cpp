#include "SettingsManager.h"

const char* configFileName = "/config.json";
const String serverName = "serverName";

Config Config2;
//Vector<Task> Tasks;
String TasksJson;

SettingsManager::SettingsManager()
{
	Serial.begin(115200);
	SPIFFS.begin();
	LoadConfig();
}

SettingsManager::~SettingsManager()
{
}

void SettingsManager::CreateEmptyConfig()
{
	auto configFileTemp = SPIFFS.open(configFileName, "w");
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["serverName"] = "initial";
	JsonArray& tasks = json.createNestedArray("tasks");

	configFileTemp = SPIFFS.open(configFileName, "w");
	if (!configFileTemp) {
		Serial.println("Failed to open config file for writing");
	}

	json.printTo(configFileTemp);
	configFileTemp.close();

	TasksJson = "";
}

void SettingsManager::LoadConfig()
{
	Config2.loaded = false;



	if (!SPIFFS.exists(configFileName)) {
		Serial.println("Creating empty file!");
		CreateEmptyConfig();
	}

	File configFile = SPIFFS.open(configFileName, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in LoadConfig)");
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

	if (!json.containsKey("tasks")) {
		json.createNestedArray("tasks");
	}
	JsonArray& tasks = json["tasks"];

	TasksJson = "";
	tasks.prettyPrintTo(TasksJson);
	Serial.println("tasks of load" + TasksJson);
	//for (int i = 0; i < tasks.size(); i++) {
	//	int h = tasks[i]["h"];
	//	int m = tasks[i]["m"];
	//	int s = tasks[i]["s"];
	//	Tasks.PushBack(Task(h, m, (s == 0) ? false : true));
	//}

	//Serial.println("Aantal taken in Vector: " + (String)Tasks.Size());
	Config2.loaded = true;
}

String SettingsManager::AddTask(Task task)
{


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
		return "";
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
	JsonArray& tasksJson = json["tasks"];

	for (int i = 0; i < tasksJson.size(); i++)
	{
		auto t = tasksJson[i];
		if ((int)t["d"] == int(task.dow)
			&& (int)t["h"] == int(task.hour)
			&& (int)t["m"] == int(task.min)
			) //&& (int)t["s"] == int(task.state)
		{
			return "Er bestaat al een taak voor het gekozen tijdstip";
		}
	}

	JsonObject& taskJson = jsonBuffer.createObject();
	taskJson["d"] = task.dow;
	taskJson["h"] = task.hour;
	taskJson["m"] = task.min;
	taskJson["s"] = task.state ? 1 : 0;
	tasksJson.add(taskJson);


	if (SPIFFS.exists(configFileName)) {
		Serial.println("Removing old config file");
		SPIFFS.remove(configFileName);
	}

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

	return "Taak succesvol aangemaakt";
}

void SettingsManager::RemoveTask(Task task)
{


	Serial.println("void SettingsManager::RemoveTask(Task task) 1");

	//CP
	if (SPIFFS.exists(configFileName)) {
		Serial.println("configFile bestaat (in RemoveTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in RemoveTask)");
	}

	File configFile = SPIFFS.open(configFileName, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in RemoveTask)");
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

	Serial.println("void SettingsManager::RemoveTask(Task task) 2");

	json["serverName"] = millis();
	JsonArray& tasksJson = json["tasks"];

	Serial.println("indexToRemove bepalen");
	int indexToRemove = -1;
	for (int i = 0; i < tasksJson.size(); i++)
	{
		auto t = tasksJson[i];
		if ((int)t["d"] == int(task.dow)
			&& (int)t["h"] == int(task.hour)
			&& (int)t["m"] == int(task.min)
			&& (int)t["s"] == int(task.state)) {
			indexToRemove = i;
			break;
		}
	}

	Serial.println("indexToRemove " + (String)indexToRemove);
	if (indexToRemove != -1) {
		tasksJson.remove(indexToRemove);
	}

	if (SPIFFS.exists(configFileName)) {
		Serial.println("Removing old config file");
		SPIFFS.remove(configFileName);
	}

	File newConfigFile = SPIFFS.open(configFileName, "w");
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

void SettingsManager::RemoveTasks()
{

	CreateEmptyConfig();
	////CP
	//if (SPIFFS.exists(configFileName)) {
	//	Serial.println("configFile bestaat (in RemoveTask)");
	//}
	//else {
	//	Serial.println("configFile bestaat niet (in RemoveTask)");
	//}

	//File configFile = SPIFFS.open(configFileName, "r");
	//if (!configFile) {
	//	Serial.println("Failed to open config file (in RemoveTask)");
	//	//   return "";
	//}

	//size_t size = configFile.size();
	//if (size > 1024) {
	//	Serial.println("Config file size is too large");
	//	//  return "";
	//}

	//// Allocate a buffer to store contents of the file.
	//std::unique_ptr<char[]> buf(new char[size]);

	//// We don't use String here because ArduinoJson library requires the input
	//// buffer to be mutable. If you don't use ArduinoJson, you may as well
	//// use configFile.readString instead.
	//configFile.readBytes(buf.get(), size);
	//configFile.close();

	//DynamicJsonBuffer jsonBuffer;
	//JsonObject& json = jsonBuffer.parseObject(buf.get());
	////CP

	//json["serverName"] = millis();
	//JsonArray& tasksJson = json["tasks"];

	//Serial.println("Aantal taken te verwijderen: " + String(tasksJson.size()));
	//for (int i = 0; i < tasksJson.size(); i++)
	//{
	//	tasksJson.remove(i);
	//	Serial.println("Taak " + String(i) + " verwijderd");
	//}

	//if (SPIFFS.exists(configFileName)) {
	//	Serial.println("Removing old config file");
	//	SPIFFS.remove(configFileName);
	//}

	//File newConfigFile = SPIFFS.open(configFileName, "w");
	//if (!newConfigFile) {
	//	Serial.println("Failed to open config file for writing");
	//	//return false;
	//}

	//Serial.println("After set");
	//json.prettyPrintTo(Serial);

	//json.printTo(newConfigFile);

	//newConfigFile.close();

	//LoadConfig();
}

String SettingsManager::MakeAlarms(OnTick_t actionOn, OnTick_t actionOff)
{


	Serial.println("void SettingsManager::RemoveTask(Task task) 1");

	//CP
	if (SPIFFS.exists(configFileName)) {
		Serial.println("configFile bestaat (in RemoveTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in RemoveTask)");
	}

	File configFile = SPIFFS.open(configFileName, "r");
	if (!configFile) {
		Serial.println("Failed to open config file (in RemoveTask)");
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

	Serial.println("void SettingsManager::RemoveTask(Task task) 2");

	json["serverName"] = millis();
	JsonArray& tasksJson = json["tasks"];

	for (int i = 0; i < 255; i++) {
		Alarm.free(i);
	}


	int aantal = 0;
	for (int i = 0; i < tasksJson.size(); i++)
	{
		auto t = tasksJson[i];
		const int d = (int)t["d"];
		const int h = (int)t["h"];
		const int m = (int)t["m"];
		const int s = (int)t["s"];

		OnTick_t action;
		if (s == 0) {
			action = actionOff;
		}
		else {
			action = actionOn;
		}

		int id;
		if (d == 8) {
			id = Alarm.alarmRepeat(h, m, (const int)0, action);
		}
		else {
			id = Alarm.alarmRepeat((timeDayOfWeek_t)d, h, m, (const int)0, action);
		}
		if (id != 255) {
			aantal++;
		}
		Serial.println(" d" + (String)d + " h" + (String)h + " m" + (String)m + " s" + (String)s + " =id:" + (String)id);
	}
	return "Alarm succesvol aangemaakt voor " + (String)aantal + " taken";
}
