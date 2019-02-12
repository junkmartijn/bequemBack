#ifndef SettingsManager_h
#define SettingsManager_h

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy
#include <FS.h>
#include <ArduinoJson.h>
//#include <Vector.h>
#include <TimeAlarms.h>


struct Config {
	String serverName;
	int port;
	bool loaded;
};

struct Task {
	Task() { dow = 0, hour = 0, min = 0, state = false; }
	Task(int h, int m, bool s) { dow = 0, hour = h, min = m, state = s; }
	Task(int d, int h, int m, bool s) { dow = d; hour = h, min = m, state = s; }
	int dow; //0=all days,1=Sunday etc
	int hour;
	int min;
	bool state;	
};

struct AA {
	AA() { type= (dtAlarmPeriod_t)0, mins = (time_t)0, id = (AlarmId)255; }
	AA(dtAlarmPeriod_t t, time_t m, AlarmId i) { type = t, mins = m, id = i; }
	dtAlarmPeriod_t type;
	time_t mins;
	AlarmId id;
};

class SettingsManager
{
public:
	SettingsManager();
	~SettingsManager();
	void AddTask(Task task);
	void RemoveTask(Task task);
	void RemoveTasks();
	Config Config2;
	//Vector<Task> Tasks;
	String TasksJson;
private:
	void LoadConfig();
};

#endif
