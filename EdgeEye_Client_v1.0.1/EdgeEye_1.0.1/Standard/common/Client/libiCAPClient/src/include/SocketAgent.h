#ifndef __SOCKETAGENT_H__
#define __SOCKETAGENT_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>

#endif

extern char serverRunning;

typedef struct{
	int sockfd;
	int length;
	char* payload;
}SOCKET_DATA;

#ifdef WIN32
DWORD WINAPI SA_Start(LPVOID portno);
#else
int SA_Start(int portno);
#endif
int SA_Stop();
int SA_CheckStatus();
int SA_SetInterval(unsigned int ms);
int SA_AddExtSensor(char* name, char* unit, unsigned char type);
int SA_AddRemoteDev(char* name, char* unit, unsigned char type);
int SA_RemoveExtSensor(char* name);
int SA_RemoveRemoteDev(char* name);
int SA_AddStaticInfo(char* name,char* info);
#ifdef WIN32
int SA_ResponseExtSensorValue(SOCKET sockfd, const char* name, double value);
int SA_ResponseRemoteOKCmd(SOCKET sockfd, const char* name);
int SA_ResponseRemoteFailCmd(SOCKET sockfd, const char* name);
#else
int SA_ResponseExtSensorValue(int sockfd, const char* name, double value);
int SA_ResponseRemoteOKCmd(int sockfd, const char* name);
int SA_ResponseRemoteFailCmd(int sockfd, const char* name);
#endif

int SA_EventTrigger();
int SA_SendLogout();

#endif
