#ifndef __DATABUILDER_HPP__
#define __DATABUILDER_HPP__

extern "C"
{
	#include "json-c/json.h"
}
#include <string>
using namespace std;
class NODE
{
	public:
	string Name;
	string Unit;
	unsigned char type;
	float last_value;
};

char* GetDeviceThumbprint();
struct json_object* GetStaticRawData();
struct json_object* GetDynamicRawData();
struct json_object* GetScreenshot(wchar_t* ImgName);
struct json_object* GetExternalSensorList();
struct json_object* GetExternalSensorValue();
struct json_object* GetRemoteDeviceList();
int AddExternalSensor(char* name, char* unit, unsigned char type);
int AddRemoteDevice(char* name, char* unit, unsigned char type);
int RemoveExternalSensor(char* name);
int RemoveRemoteDevice(char* name);
void ClearExternalInformation();
int CheckInnodisk();
void CheckDataGroup();
void CheckData();
void GetLocalStorageInfo();
void GetLocalDynamicRawData(void* local_ptr);
void GetSysInfo();
void CheckSysInfo();
void FreeSysInfo();

#endif
