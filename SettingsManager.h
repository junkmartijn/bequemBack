#ifndef SettingsManager_h
#define SettingsManager_h

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy
#include <FS.h>
#include <ArduinoJson.h>
//#include <Vector.h>
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>


struct Config {
	String serverName;
	int port;
	bool loaded;
};

struct Task {
	Task() { dow = 0, hour = 0, min = 0, state = false; }
	Task(int d, int h, int m, bool s) { dow = d; hour = h, min = m, state = s; }
	int dow; //1=Sunday etc, 8=all days
	int hour;
	int min;
	bool state;	
};

class SettingsManager
{
public:
	SettingsManager();
	~SettingsManager();
	void CreateEmptyConfig();
	String AddTask(Task task);
	void RemoveTask(Task task);
	void RemoveTasks();
	String MakeAlarms(void action_on(), void action_off());
	Config Config2;
	//Vector<Task> Tasks;
	String TasksJson;
private:
	void LoadConfig();
};

#endif
