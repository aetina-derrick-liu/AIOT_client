#ifndef __SOCKETAGENT_HPP__
#define __SOCKETAGENT_HPP__


#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
#else
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

typedef struct{
#ifdef WIN32
	SOCKET sockfd;
#else
	int sockfd;
#endif
	int length;
	char* payload;
}SOCKET_DATA;


int SA_Start(int portno);
void SA_Stop();
int SA_SendPackages(SOCKET_DATA* ptr, const char* payload);
int SA_SendPackages_Bytes(SOCKET_DATA* ptr, const char* payload, int length);
void SA_DestroySocketObject(SOCKET_DATA* ptr);
int SA_SendOKCmd(SOCKET_DATA* ptr);
int SA_SendFailCmd(SOCKET_DATA* ptr);
int SA_SendStatus(SOCKET_DATA* ptr);
float SA_GetExtSensorValue(char* name);
int SA_SendRemoteCmd(char* name, char* value);

#endif
