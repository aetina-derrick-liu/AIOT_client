#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "JsonParser.h"
#include "SocketAgent.h"
#include "DeviceAgent.h"

char serverRunning = 0;

#ifndef WIN32
pthread_t pth_server;
#endif

int SendCommandToService(const char* cmd, char* ret)
{
#ifdef WIN32
	WSADATA wsaData;
	SOCKET sockfd = INVALID_SOCKET;
	struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    struct sockaddr_in server;
#else
	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
#endif

	int n;
	char buffer[256];

#ifdef WIN32
	// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    if((sockfd = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 2888 );

    //Connect to remote server
    if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    n = send(sockfd, cmd, strlen(cmd), 0);
    ZeroMemory(buffer,256);
    n = recv(sockfd, buffer, 255, 0);
	if (n < 0)
	{
 		fprintf(stderr,"ERROR reading from socket");
		return -4;
	}

	closesocket(sockfd);
#else
	portno = 2888;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		fprintf(stderr,"ERROR opening socket");
		return -1;
	}
	server = gethostbyname("localhost");
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		return -2;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		fprintf(stderr,"ERROR connecting");
		return -3;
	}
	n = write(sockfd, cmd,strlen(cmd));
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	if (n < 0)
	{
 		fprintf(stderr,"ERROR reading from socket");
		return -4;
	}
	close(sockfd);
#endif

	memcpy(ret, buffer, n);
	return 0;
}

#ifdef WIN32
int ResponseCommandToService(SOCKET sockfd, const char* cmd)
#else
int ResponseCommandToService(int sockfd, const char* cmd)
#endif
{
	int n;

#ifdef WIN32
	n = send(sockfd, cmd, strlen(cmd), 0);
#else
	n = write(sockfd, cmd, strlen(cmd));
#endif
	if(n < 0)
	{
		fprintf(stderr, "ERROR send cmd to socked\n");
		return -1;
	}

#ifdef WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif
	return n;
}

#ifndef WIN32
void StartListening(void* sockfdptr)
{
	int sockfd, clientfd, len;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	fd_set rfds;
	struct timeval tv;
	char buffer[65536];

	sockfd = *((int*)sockfdptr);
	free(sockfdptr);

	printf("Start listening port 2889, sockfd=%d\n", sockfd);

	while(serverRunning == 1)
	{
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(sockfd + 1, &rfds, NULL, NULL, &tv);
		if(FD_ISSET(sockfd, &rfds))
		{
			clientfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
			if(clientfd < 0)
			{
				fprintf(stderr, "Error on accept.\n");
				continue;
			}
			bzero(buffer, 65536);
			len = read(clientfd, buffer, 65536);
			if(len < 0)
			{
				fprintf(stderr, "Error reading from socket.\n");
				continue;
			}
			SOCKET_DATA* ptr = (SOCKET_DATA*)malloc(sizeof(SOCKET_DATA));
			ptr->payload = (char*)malloc(sizeof(char) * (len + 1));
			ptr->length = len;
			ptr->sockfd = clientfd;
			memcpy(ptr->payload, buffer, len);

			pthread_t pth;
			pthread_create(&pth, NULL, (void*)DA_GetServiceCmd, (void*)ptr);
		}
	}
	close(sockfd);
	serverRunning = 0;
}
#endif

#ifdef WIN32
DWORD WINAPI SA_Start(LPVOID portno)
#else
int SA_Start(int portno)
#endif
{
#ifdef WIN32
	WSADATA wsaData;
    int iResult;

    int c, len;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    SOCKET* sockfdptr = malloc(sizeof(ListenSocket));

    struct sockaddr_in server, client;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Resolve the server address and port
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 2889 );

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, (struct sockaddr *)&server , sizeof(server));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Start listening socket
    iResult = listen(ListenSocket, 10);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    *sockfdptr = ListenSocket;

	serverRunning = 1;

	while( (ClientSocket = accept(ListenSocket , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
    {
        char buffer[1024];
        recv(ClientSocket, buffer, 1024, 0);
		DA_GetServiceCmd(ClientSocket, buffer);
    }
#else
	int sockfd;
	struct sockaddr_in serv_addr;
	int* sockfdptr = malloc(sizeof(sockfdptr));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Error on opening socket.\n");
		return -1;
	}

	bzero((char*)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Error on binding.\n");
		return -2;
	}
	listen(sockfd, 5);
	serverRunning = 1;
	
	*sockfdptr = sockfd;

	pthread_create(&pth_server, NULL, (void*)&StartListening, sockfdptr);
#endif

	return 0;
}

int SA_Stop()
{
	serverRunning = 2;
#ifndef WIN32
	while(serverRunning != 0)
	{
		usleep(1000000L);
	}
#endif
	return 0;
}

int SA_CheckStatus()
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("ChkStatus"));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "ChkStatus") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			ret = json_object_get_int(retobj);
		}
	}
	free(retCmd);
	return ret;
}

int SA_SetInterval(unsigned int ms)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("SetInt"));
	json_object_object_add(jobj, "Value", json_object_new_int(ms));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "SetInt") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			ret = -1;
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
		}
	}
	free(retCmd);
	return ret;	
}

int SA_AddExtSensor(char* name, char* unit, unsigned char type)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("AddExt"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Unit", json_object_new_string(unit));
	json_object_object_add(jobj, "Type", json_object_new_int(type));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "AddExt") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		json_object_put(jobj);
	}
	free(retCmd);
	return ret;
}

int SA_AddStaticInfo(char* name,char* info)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("AddStatic"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Info", json_object_new_string(info));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "AddStatic") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			ret = -1;
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
		}
	}
	free(retCmd);
	return ret;	
}

int SA_AddRemoteDev(char* name, char* unit, unsigned char type)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("AddRemote"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Unit", json_object_new_string(unit));
	json_object_object_add(jobj, "Type", json_object_new_int(type));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "AddRemote") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		json_object_put(jobj);
	}
	free(retCmd);
	return ret;
}

int SA_RemoveExtSensor(char* name)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("DelExt"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "DelExt") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		json_object_put(jobj);
	}
	free(retCmd);
	return ret;
}

int SA_RemoveRemoteDev(char* name)
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("DelRemote"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "DelRemote") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		json_object_put(jobj);
	}
	free(retCmd);
	return ret;
}

#ifdef WIN32
int SA_ResponseExtSensorValue(SOCKET sockfd, const char* name, double value)
#else
int SA_ResponseExtSensorValue(int sockfd, const char* name, double value)
#endif
{
	int ret;
	struct json_object* jobj;
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("GetExt"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Value", json_object_new_double(value));

	ret = ResponseCommandToService(sockfd, json_object_to_json_string(jobj));
	json_object_put(jobj);
	return ret;
}

#ifdef WIN32
int SA_ResponseRemoteOKCmd(SOCKET sockfd, const char* name)
#else
int SA_ResponseRemoteOKCmd(int sockfd, const char* name)
#endif
{
	int ret;
	struct json_object* jobj;
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("Remote"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Ret", json_object_new_string("ok"));

	ret = ResponseCommandToService(sockfd, json_object_to_json_string(jobj));
	json_object_put(jobj);
	return ret;
}

#ifdef WIN32
int SA_ResponseRemoteFailCmd(SOCKET sockfd, const char* name)
#else
int SA_ResponseRemoteFailCmd(int sockfd, const char* name)
#endif
{
	int ret;
	struct json_object* jobj;
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("Remote"));
	json_object_object_add(jobj, "Name", json_object_new_string(name));
	json_object_object_add(jobj, "Ret", json_object_new_string("fail"));

	ret = ResponseCommandToService(sockfd, json_object_to_json_string(jobj));
	json_object_put(jobj);
	return ret;
}

int SA_EventTrigger()
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("EventTrigger"));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "EventTrigger") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			ret = -1;
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
		}
	}
	free(retCmd);
	return ret;	
}

int SA_SendLogout()
{
	int ret;
	struct json_object* jobj, *cmd, *retobj;
	char* retCmd = (char*)malloc(sizeof(char)*255);
	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("ServiceLogout"));

	ret = SendCommandToService(json_object_to_json_string(jobj), retCmd);
	
	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retCmd);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "ServiceLogout") == 0)
		{
			retobj = get_json_object(jobj, "Ret");
			ret = -1;
			if(strcmp(json_object_get_string(retobj), "ok") == 0)
			{
				ret = 0;
			}
		}
	}
	free(retCmd);
	return ret;	
}
