#ifndef __DEVICEAGENT_H__
#define __DEVICEAGENT_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "libiCAPClient.h"

typedef struct
{
	void* prev_node;
	char* name;
	GetValueFunc func;
	void* next_node;
}ExtSensorList;

typedef struct
{
	void* prev_node;
	char* name;
	SendRemoteFunc func;
	void* next_node;
}RemoteDeviceList;

int DA_AddExtSensorToList(char* name, GetValueFunc func);
int DA_RemoveExtSensor(char* name);
double DA_GetExtSensorValue(const char* name);
int DA_AddRemoteDeviceToList(char* name, SendRemoteFunc func);
int DA_RemoveRemoteDevice(char* name);
int DA_SendRemoteDeviceCmd(const char* name, void* value);
#ifdef WIN32
void DA_GetServiceCmd(SOCKET client_sock, char* payload);
#else
void* DA_GetServiceCmd(void* sockdata);
#endif

#endif
