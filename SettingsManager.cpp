#include "SettingsManager.h"

const char* config_file_name = "/config.json";
const String server_name = "serverName";

Config config2;
//Vector<Task> Tasks;
String tasks_json;

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
	auto config_file_temp = SPIFFS.open(config_file_name, "w");
	StaticJsonBuffer<200> json_buffer;
	JsonObject& json = json_buffer.createObject();
	json[server_name] = "initial";
	auto& tasks = json.createNestedArray("tasks");

	config_file_temp = SPIFFS.open(config_file_name, "w");
	if (!config_file_temp) {
		Serial.println("Failed to open config file for writing");
	}

	json.printTo(config_file_temp);
	config_file_temp.close();

	TasksJson = "";
}

void SettingsManager::LoadConfig()
{
	Config2.loaded = false;



	if (!SPIFFS.exists(config_file_name)) {
		Serial.println("Creating empty file!");
		CreateEmptyConfig();
	}

	auto config_file = SPIFFS.open(config_file_name, "r");
	if (!config_file) {
		Serial.println("Failed to open config file (in LoadConfig)");
	}

	size_t size = config_file.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	config_file.readBytes(buf.get(), size);

	DynamicJsonBuffer json_buffer;
	auto& json = json_buffer.parseObject(buf.get());

	if (!json.success()) {
		Serial.println("Failed to parse config file");
		return;
	}
	Serial.print("json: ");
	json.prettyPrintTo(Serial);

	const char* val = json[server_name];
	Config2.serverName = val;

	if (!json.containsKey("tasks")) {
		json.createNestedArray("tasks");
	}
	JsonArray& tasks = json["tasks"];

	TasksJson = "";
	tasks.prettyPrintTo(TasksJson);
	Serial.println("tasks of load" + TasksJson);
	Config2.loaded = true;
}

String SettingsManager::AddTask(Task task)
{


	//CP
	if (SPIFFS.exists(config_file_name)) {
		Serial.println("configFile bestaat (in AddTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in AddTask)");
	}

	auto config_file = SPIFFS.open(config_file_name, "r");
	if (!config_file) {
		Serial.println("Failed to open config file (in AddTask)");
		//   return "";
	}

	size_t size = config_file.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		return "";
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	config_file.readBytes(buf.get(), size);
	config_file.close();

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());
	//CP

	json[server_name] = millis();
	JsonArray& tasks_json = json["tasks"];

	for (int i = 0; i < tasks_json.size(); i++)
	{
		auto t = tasks_json[i];
		if ((int)t["d"] == int(task.dow)
			&& (int)t["h"] == int(task.hour)
			&& (int)t["m"] == int(task.min)
			) //&& (int)t["s"] == int(task.state)
		{
			return "Er bestaat al een taak voor het gekozen tijdstip";
		}
	}

	auto& taskJson = jsonBuffer.createObject();
	taskJson["d"] = task.dow;
	taskJson["h"] = task.hour;
	taskJson["m"] = task.min;
	taskJson["s"] = task.state ? 1 : 0;
	tasks_json.add(taskJson);


	if (SPIFFS.exists(config_file_name)) {
		Serial.println("Removing old config file");
		SPIFFS.remove(config_file_name);
	}

	auto new_config_file = SPIFFS.open("/config.json", "w");
	if (!new_config_file) {
		Serial.println("Failed to open config file for writing");
		//return false;
	}

	Serial.println("After set");
	json.prettyPrintTo(Serial);

	json.printTo(new_config_file);

	new_config_file.close();

	LoadConfig();

	return "Taak succesvol aangemaakt";
}

void SettingsManager::RemoveTask(Task task)
{


	Serial.println("void SettingsManager::RemoveTask(Task task) 1");

	//CP
	if (SPIFFS.exists(config_file_name)) {
		Serial.println("configFile bestaat (in RemoveTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in RemoveTask)");
	}

	auto configFile = SPIFFS.open(config_file_name, "r");
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

	DynamicJsonBuffer json_buffer;
	auto& json = json_buffer.parseObject(buf.get());
	//CP

	Serial.println("void SettingsManager::RemoveTask(Task task) 2");

	json[server_name] = millis();
	JsonArray& tasks_json = json["tasks"];

	Serial.println("indexToRemove bepalen");
	auto indexToRemove = -1;
	for (auto i = 0; i < tasks_json.size(); i++)
	{
		auto t = tasks_json[i];
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
		tasks_json.remove(indexToRemove);
	}

	if (SPIFFS.exists(config_file_name)) {
		Serial.println("Removing old config file");
		SPIFFS.remove(config_file_name);
	}

	auto new_config_file = SPIFFS.open(config_file_name, "w");
	if (!new_config_file) {
		Serial.println("Failed to open config file for writing");
		//return false;
	}




	Serial.println("After set");
	json.prettyPrintTo(Serial);

	json.printTo(new_config_file);

	new_config_file.close();

	LoadConfig();
}

void SettingsManager::RemoveTasks()
{
	CreateEmptyConfig();
}

String SettingsManager::MakeAlarms(void action_on(), void action_off())
{
	Serial.println("void SettingsManager::RemoveTask(Task task) 1");

	//CP
	if (SPIFFS.exists(config_file_name)) {
		Serial.println("configFile bestaat (in RemoveTask)");
	}
	else {
		Serial.println("configFile bestaat niet (in RemoveTask)");
	}

	auto config_file = SPIFFS.open(config_file_name, "r");
	if (!config_file) {
		Serial.println("Failed to open config file (in RemoveTask)");
		//   return "";
	}

	size_t size = config_file.size();
	if (size > 1024) {
		Serial.println("Config file size is too large");
		//  return "";
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	config_file.readBytes(buf.get(), size);
	config_file.close();

	DynamicJsonBuffer json_buffer;
	JsonObject& json = json_buffer.parseObject(buf.get());
	//CP

	Serial.println("void SettingsManager::RemoveTask(Task task) 2");

	json[server_name] = millis();
	JsonArray& tasks_json = json["tasks"];

	for (int i = 0; i < 255; i++) {
		Alarm.free(i);
	}


	auto aantal = 0;
	for (auto i = 0; i < tasks_json.size(); i++)
	{
		auto t = tasks_json[i];
		const auto d = (int)t["d"];
		const auto h = (int)t["h"];
		const auto m = (int)t["m"];
		const auto s = (bool)t["s"];

		int id;
		if (d == 8) {
			id = Alarm.alarmRepeat(h, m, 0, s ? action_on : action_off);
		}
		else {
			id = Alarm.alarmRepeat((timeDayOfWeek_t)d, h, m, (const int)0, s ? action_on : action_off);
		}
		if (id != 255) {
			aantal++;
		}
		Serial.println(" d" + (String)d + " h" + (String)h + " m" + (String)m + " s" + (String)s + " =id:" + (String)id);
	}
	return "Alarm succesvol aangemaakt voor " + (String)aantal + " taken";
}