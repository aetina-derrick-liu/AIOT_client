#include <stdio.h>
#include <pthread.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include "main.hpp"
#include "SocketAgent.hpp"
#include "DeviceAgent.hpp"
#include "JsonParser.hpp"
#include "LogAgent.hpp"
#include "WebService.hpp"

#ifdef WIN32
#pragma comment (lib, "Ws2_32.lib")
#endif

char serverRunning = 0;

int SendCommandToDevice(const char* cmd, char* ret)
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
	char buffer[2048] = {0};

#ifdef WIN32
	// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
#ifdef DEBUG
        LogAgent_Write("WSAStartup failed with error: %d", iResult);
#endif
        return 1;
    }

    if((sockfd = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
#ifdef DEBUG
        LogAgent_Write("Could not create socket : %d" , WSAGetLastError());
#endif
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 2889 );

    //Connect to remote server
    if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    n = send(sockfd, cmd, strlen(cmd), 0);
    ZeroMemory(buffer,2048);
    n = recv(sockfd, buffer, 255, 0);
	if (n < 0)
	{
 		fprintf(stderr,"ERROR reading from socket");
		return -4;
	}

	closesocket(sockfd);
#else
	portno = 2889;
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
SOCKET GetListeningSocket(int portnumber)
#else
int GetListeningSocket(int portnumber)
#endif
{
#ifdef WIN32
  WSADATA wsaData;
  SOCKET ListenSocket = INVALID_SOCKET;
#else
  int reuseAddr = 1;
  int ListenSocket;
#endif
  int iResult;

  struct sockaddr_in server;

#ifdef WIN32
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    return INVALID_SOCKET;
  }
#endif

  // Create a SOCKET for connecting to server
  ListenSocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
  if (ListenSocket == INVALID_SOCKET) {
#ifdef WIN32
    WSACleanup();
#endif
    return INVALID_SOCKET;
  }

#ifndef WIN32
	iResult = setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));
#endif

  // Resolve the server address and port
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(portnumber);

  iResult = bind( ListenSocket, (struct sockaddr *)&server , sizeof(server));
  if (iResult == SOCKET_ERROR) {
#ifdef WIN32
    closesocket(ListenSocket);
    WSACleanup();
#else
    close(ListenSocket);
#endif
    return INVALID_SOCKET;
  }

  // Start listening socket
  iResult = listen(ListenSocket, 10);
  if (iResult == SOCKET_ERROR) {
#ifdef WIN32
    closesocket(ListenSocket);
    WSACleanup();
#else
    close(ListenSocket);
#endif
    return INVALID_SOCKET;
  }
  
  return ListenSocket; 
}

void* ProcessPackage(void* socket_data_object)
{
	SOCKET_DATA* ptr = (SOCKET_DATA*)socket_data_object;

    if(strncmp(ptr->payload, "GET", 3) == 0)
    {
#if 0
    	LogAgent_Write("Get HTTP GET request:%s", ptr->payload);
#endif
    	WEB_GETRequest(ptr);
    }
    else if(strncmp(ptr->payload, "POST", 4) == 0)
    {
#if 0
    	LogAgent_Write("Get HTTP POST request:%s", ptr->payload);
#endif
    	WEB_POSTRequest(ptr);
    }
    else if(strncmp(ptr->payload, "PUT", 3) == 0)
    {
#if 0
    	LogAgent_Write("Get HTTP PUT request:%s", ptr->payload);
#endif
    	WEB_PUTRequest(ptr);
    }
    else
    {
#ifdef DEBUG
    	LogAgent_Write("Get device command:%s", ptr->payload);
#endif
		DA_receiveDeviceCommand(ptr);
	}
	pthread_exit((void*)"thread killed,return!"); 
}

#ifdef WIN32
DWORD WINAPI StartListening(LPVOID sockfdobj)
#else
void* StartListening(void* sockfdobj)
#endif
{
	pthread_detach(pthread_self());
	fd_set rfds;
	struct timeval tv;
	int bytesReceived;

	pthread_t pth;
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	void *a1;

#ifdef WIN32
	SOCKET sockfd = (SOCKET)sockfdobj;
	SOCKET ClientSocket;
#else
	int sockfd = *((int*)sockfdobj);
	int ClientSocket;
	struct sockaddr_in * addr = (sockaddr_in*)malloc(sizeof *addr);
	socklen_t clilen=sizeof*addr;
	
#endif
	while(serverRunning == 1)
	{
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(sockfd + 1, &rfds, NULL, NULL, &tv);
		if(FD_ISSET(sockfd, &rfds))
		{
#ifdef WIN32
			ClientSocket = accept((SOCKET)sockfd , NULL, NULL);
			if(ClientSocket == INVALID_SOCKET)
#else
			ClientSocket = accept(sockfd, (struct sockaddr*)addr,&clilen);
			if(ClientSocket < 0)
#endif
			{
#ifdef DEBUG
				LogAgent_Write("Error on accept.");
#endif
				continue;
			}

			char* buffer = (char*)malloc(sizeof(char) * 65536);
			char* realloc_buffer=NULL;
			memset(buffer, 0, 65536);

#ifdef WIN32
			bytesReceived = recv(ClientSocket, buffer, 65536, 0);
#else
			bytesReceived = read(ClientSocket, buffer, 65536);
#endif
			if(bytesReceived <= 0)
			{
#ifdef DEBUG
				LogAgent_Write("Error reading from socket.");
#endif
				continue;
			}
			realloc_buffer = (char*)realloc(buffer, sizeof(char) * bytesReceived);
			if(realloc_buffer == NULL)
			{
#ifdef DEBUG
				LogAgent_Write("Error on realloc buffer, buffer=%s\n.", buffer);
#endif
				continue;
			}

			SOCKET_DATA* ptr = (SOCKET_DATA*)malloc(sizeof(SOCKET_DATA));
			ptr->payload = buffer = realloc_buffer;
			ptr->length = bytesReceived;

			ptr->sockfd = ClientSocket;
			pthread_create(&pth, &attr, ProcessPackage, (void*)ptr);
		}
	}
#ifdef WIN32
	closesocket(sockfd);
#else
	close(sockfd);
	free(addr);
#endif
	pthread_attr_destroy (&attr);
	serverRunning = 0;
}

int SA_Start(int portno)
{
#ifdef WIN32	
	SOCKET sockfd;
#else
	static int sockfd;
#endif

	if(serverRunning == 1)
	{
		LogAgent_Write((char*)"Socket agent already in running status.");
		return 1;
	}

	sockfd = GetListeningSocket(portno);

	serverRunning = 1;

#ifdef WIN32
	if(sockfd != INVALID_SOCKET)
	{
		CreateThread(NULL, 0, StartListening, (LPVOID)sockfd, 0, 0);
	}
#else
	if(sockfd >= 0)
	{
		pthread_t pth_server;
		pthread_attr_t attr;
		pthread_attr_init (&attr);
		pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&pth_server, &attr, &StartListening, &sockfd);
		pthread_attr_destroy (&attr);
	}
#endif

	return 0;
}

void SA_Stop()
{
	serverRunning = 2;
#ifndef WIN32
	while(serverRunning != 0)
	{
		usleep(1000000L);
	}
#endif
}

int SA_SendPackages(SOCKET_DATA* ptr, const char* payload)
{
	int sendLength;
#ifdef WIN32
	sendLength = send(ptr->sockfd, payload, strlen(payload), 0);
#else
	sendLength = write(ptr->sockfd, payload, strlen(payload));
#endif
	return sendLength;
}

int SA_SendPackages_Bytes(SOCKET_DATA* ptr, const char* payload, int length)
{
	int sendLength;
#ifdef WIN32
	sendLength = send(ptr->sockfd, payload, length, 0);
#else
	sendLength = write(ptr->sockfd, payload, length);
#endif
	return sendLength;
}

void SA_DestroySocketObject(SOCKET_DATA* ptr)
{
#ifdef WIN32
	closesocket(ptr->sockfd);
#else
	close(ptr->sockfd);
#endif
	free(ptr->payload);
	free(ptr);
}

int SA_SendOKCmd(SOCKET_DATA* ptr)
{

	int n;
	//const char *cmdStr, *sendString;
	char sendString[1024];
	struct json_object* jobj, *cmd, *ret;

	printf("Send ok command, ptr->payload:%s\n", ptr->payload);

	sprintf(sendString,"{\"Cmd\":\"AddExt\",\"Ret\":\"ok\"}");

#ifdef DEBUG
	LogAgent_Write("---Send raw---");
	LogAgent_Write("%s", (char*)sendString);
	LogAgent_Write("--------------");
	LogAgent_Write("Length: %d", (int)strlen(sendString));
	LogAgent_Write("---- Done ----");
#endif

	n = SA_SendPackages(ptr, sendString);
	printf("8\n");

	if(n > 0)
	{
		n = 0;
	}

	SA_DestroySocketObject(ptr);


	return n;
}

int SA_SendFailCmd(SOCKET_DATA* ptr)
{
	struct json_object *jobj, *cmd, *ret;
	const char *cmdStr, *sendString;
	int n;
	
	printf("Send fail command, ptr->payload:%s\n", ptr->payload);

	jobj = json_tokener_parse(ptr->payload);

	cmd = get_json_object(jobj, "Cmd");
	ret = json_object_new_object();
	cmdStr = json_object_get_string(cmd);
	json_object_object_add(ret, "Cmd", json_object_new_string(cmdStr));
	json_object_object_add(ret, "Ret", json_object_new_string("fail"));
	sendString = json_object_to_json_string(ret);

#ifdef DEBUG
	LogAgent_Write("---Send raw---");
	LogAgent_Write("%s", (char*)sendString);
	LogAgent_Write("--------------");
	LogAgent_Write("Length: %d", (int)strlen(sendString));
	LogAgent_Write("---- Done ----");
#endif

	n = SA_SendPackages(ptr, sendString);

	if(n > 0)
	{
		n = 0;
	}

	json_object_put(jobj);
	json_object_put(ret);

	SA_DestroySocketObject(ptr);
	return n;
}

int SA_SendStatus(SOCKET_DATA* ptr)
{
	struct json_object *jobj, *cmd, *ret;
	const char *cmdStr, *sendString;
	int n;

	jobj = json_tokener_parse(ptr->payload);

	cmd = get_json_object(jobj, "Cmd");
	ret = json_object_new_object();
	cmdStr = json_object_get_string(cmd);
	json_object_object_add(ret, "Cmd", json_object_new_string(cmdStr));
	json_object_object_add(ret, "Ret", json_object_new_int(ServiceStatus));
	sendString = json_object_to_json_string(ret);

#ifdef DEBUG
	LogAgent_Write("---Send raw---");
	LogAgent_Write("%s", (char*)sendString);
	LogAgent_Write("--------------");
	LogAgent_Write("Length: %d", (int)strlen(sendString));
	LogAgent_Write("---- Done ----");
#endif

	n = SA_SendPackages(ptr, sendString);

	if(n > 0)
	{
		n = 0;
	}

	json_object_put(jobj);
	json_object_put(ret);

	SA_DestroySocketObject(ptr);
	return n;
}

float SA_GetExtSensorValue(char* name)
{	
	int ret;
	float retValue = 0;
	struct json_object *jobj, *cmd, *nameobj, *value;
	char* retStr = (char*)malloc(sizeof(char) * 2048);

	memset(retStr, 0, 2048);

	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("GetExt"));	
	json_object_object_add(jobj, "Name", json_object_new_string(name));	

	ret = SendCommandToDevice(
			json_object_to_json_string(jobj),
			retStr);

	json_object_put(jobj);

	if(ret == 0)
	{
		jobj = json_tokener_parse(retStr);
		cmd = get_json_object(jobj, "Cmd");
		if(json_object_get_string(cmd)!="")
		{
			if(strcmp(json_object_get_string(cmd), "GetExt") == 0)
			{
				nameobj = get_json_object(jobj, "Name");
				if(strcmp(json_object_get_string(nameobj), name) == 0)
				{
					value = get_json_object(jobj, "Value");
					retValue = (float)json_object_get_double(value);
				}
			}
		}
		json_object_put(jobj);
	}

	free(retStr);
	return retValue;
}

int SA_SendRemoteCmd(char* name, char* value)
{
	int ret;
	float retValue = 0;
	struct json_object *jobj, *cmd, *nameobj, *valueobj;
	char* retStr = (char*)malloc(sizeof(char) * 255);

	jobj = json_object_new_object();
	json_object_object_add(jobj, "Cmd", json_object_new_string("Remote"));	
	json_object_object_add(jobj, "Name", json_object_new_string(name));	
	json_object_object_add(jobj, "Value", json_object_new_string(value));

	ret = SendCommandToDevice(
			json_object_to_json_string(jobj),
			retStr);

	json_object_put(jobj);	

	if(ret == 0)
	{
		jobj = json_tokener_parse(retStr);
		cmd = get_json_object(jobj, "Cmd");
		if(strcmp(json_object_get_string(cmd), "Remote") == 0)
		{
			nameobj = get_json_object(jobj, "Name");
			if(strcmp(json_object_get_string(nameobj), name) == 0)
			{
				valueobj = get_json_object(jobj, "Ret");
				ret = -1;
				if(strcmp(json_object_get_string(valueobj), "ok") == 0)
				{
					ret = 0;
				}
			}
		}
		json_object_put(jobj);
	}

	free(retStr);
	return ret;
}

