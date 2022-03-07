#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "WebService.hpp"
#include "DBInterface.hpp"
#include "ClientService.hpp"
#include "LogAgent.hpp"
#include "Base64.hpp"
#include "JsonParser.hpp"



enum {DashboardData = 0, DeviceLocation, DeviceSetting, ThresholdSetting, UnreadLog};

char* (*GetDataFunctionPointer[5])() = {
	GetDashobardData,
	GetDeviceLocation,
	GetDeviceSetting,
	GetThresholdSetting,
	GetUnreadLogBaner
};

HEADER *curr_header;
char* ConvertToLowerCase(char* target, int length)
{
	int i;
	
	for(i = 0; i < length; i++)
	{
		*(target+i) = (char)tolower(*(target+i));
	}

	return target;
}

void ResponseOKPackage(SOCKET_DATA* sock_obj, int type)
{
	char* ret = NULL;

	ret = GetDataFunctionPointer[type]();

	SA_SendPackages(sock_obj, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");

	if(ret)
	{
		if(strlen(ret) > 0)
		{
			SA_SendPackages(sock_obj, ret);
		}
	}	
		
	SA_SendPackages(sock_obj, "\r\n");

	free(ret);
}

void ResponseNotSupport(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"Method not support\"}\r\n");
}
void ResponseBattIDNotMatch(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 403 Not Found\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"Battery ID does not match\"}\r\n");
}

void ResponseTokenFailed(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 403 Not Found\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"The identity token not found\"}\r\n");
}

void ResponseNotImplement(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"Method not implemented\"}\r\n");
}

void ResponseAccept(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 202 Accepted\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"Ok\"}\r\n");
}

void ResponseNotAccept(SOCKET_DATA* sock_obj)
{
	SA_SendPackages(sock_obj, "HTTP/1.0 406 Not Accepted\r\nContent-Type: text/plain\r\n\r\n{\"Response\":\"Fail\"}\r\n");
}

void ResponseLogData(SOCKET_DATA* sock_obj, int DataCount)
{
	char* ret = NULL;

	ret = GetLogData(DataCount);

	SA_SendPackages(sock_obj, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");

	if(ret)
	{
		if(strlen(ret) > 0)
		{
			SA_SendPackages(sock_obj, ret);
		}
	}	
		
	SA_SendPackages(sock_obj, "\r\n");

	free(ret);
}

void ResponseUnreadLogCount(SOCKET_DATA* sock_obj)
{
	char buffer[100] = {0};
	int ret;

	ret = GetUnreadLogCount();
	
	SA_SendPackages(sock_obj, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");

	sprintf(buffer,"%d\r\n", ret);
		
	SA_SendPackages(sock_obj, buffer);
}

void UpdateDeviceSetting(SOCKET_DATA* sock_obj)
{
	int i = 0;
	SETTING_DEVICE* set_dev_payload = (SETTING_DEVICE*)malloc(sizeof(SETTING_DEVICE));
	memset(set_dev_payload,0,sizeof(SETTING_DEVICE));

	int tmpIndex = 0, nextIndex;
	char target[2048] = {0};
	char key[1024] = {0};
	char value[1024] = {0};

	while(*(sock_obj->payload + tmpIndex + 1) != '\0')
	{
		memset(target, 0, 2048);
		memset(key, 0, 1024);
		memset(value, 0, 1024);
		nextIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
		strncpy(target, sock_obj->payload + tmpIndex + 1, nextIndex - tmpIndex);
		sscanf(target, "%s %s", key, value);
		ConvertToLowerCase(key, strlen(key));
		
		if(strncmp(key, "alias", 5) == 0)
		{
			set_dev_payload->Alias = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->Alias, "%s", value);
		}
		else if(strncmp(key, "serverip", 8) == 0)
		{
			set_dev_payload->ServerIp = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->ServerIp, "%s", value);
		}
		else if(strncmp(key, "uploadinterval", 14) == 0)
		{
			set_dev_payload->UploadInterval = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "datalimitcount", 14) == 0)
		{
			set_dev_payload->DataLimitCount = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "eventresendinterval", 19) == 0)
		{
			set_dev_payload->ResendInterval = strtol(value, NULL, 0);
		}

		tmpIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
	}

#ifdef DEBUG
	printf("%s,%s,%d,%d,%d\n", set_dev_payload->Alias
						  , set_dev_payload->ServerIp
						  , set_dev_payload->UploadInterval
						  , set_dev_payload->DataLimitCount
						,set_dev_payload->ResendInterval);
#endif

	i = SetDeviceParameter(set_dev_payload);

	free(set_dev_payload->Alias);
	free(set_dev_payload->ServerIp);
	free(set_dev_payload);

	if(i < 0)
	{
		ResponseNotAccept(sock_obj);
		return;
	}
	ResponseAccept(sock_obj);

	ClientService_Restart();
}
#ifdef iON
void Update_iON_DeviceSetting(SOCKET_DATA* sock_obj)
{
	int i = 0;
	SETTING_iON_DEVICE* set_dev_payload = (SETTING_iON_DEVICE*)malloc(sizeof(SETTING_iON_DEVICE));
	memset(set_dev_payload,0,sizeof(SETTING_iON_DEVICE));

	int tmpIndex = 0, nextIndex;
	char target[2048] = {0};
	char key[1024] = {0};
	char value[1024] = {0};

	while(*(sock_obj->payload + tmpIndex + 1) != '\0')
	{
		memset(target, 0, 2048);
		memset(key, 0, 1024);
		memset(value, 0, 1024);
		nextIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
		strncpy(target, sock_obj->payload + tmpIndex + 1, nextIndex - tmpIndex);
		sscanf(target, "%s %s", key, value);
		ConvertToLowerCase(key, strlen(key));
		
		if(strncmp(key, "alias", 5) == 0)
		{
			set_dev_payload->Alias = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->Alias, "%s", value);
		}
		else if(strncmp(key, "serverip", 8) == 0)
		{
			set_dev_payload->ServerIp = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->ServerIp, "%s", value);
		}
		else if(strncmp(key, "uploadinterval", 14) == 0)
		{
			set_dev_payload->UploadInterval = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "account", 7) == 0)
		{
			set_dev_payload->Account = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->Account, "%s", value);
		}
		else if(strncmp(key, "pwd", 3) == 0)
		{
			set_dev_payload->Pwd = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->Pwd, "%s", value);
		}
		else if(strncmp(key, "ezproip", 7) == 0)
		{
			set_dev_payload->EZPROIP = (char*)malloc(sizeof(char) * strlen(value) + 1);
			sprintf(set_dev_payload->EZPROIP, "%s", value);
		}
		else if(strncmp(key, "port", 4) == 0)
		{
			set_dev_payload->Port = strtol(value, NULL, 0);
		}

		tmpIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
	}

#ifdef DEBUG
	printf("%s,%s,%d,%s,%s,%s,%d\n", set_dev_payload->Alias
						  , set_dev_payload->ServerIp
						  , set_dev_payload->UploadInterval
						  , set_dev_payload->Account
						  , set_dev_payload->Pwd
						  , set_dev_payload->EZPROIP
						  , set_dev_payload->Port
						);
#endif

	i = Set_iON_DeviceParameter(set_dev_payload);

	free(set_dev_payload->Alias);
	free(set_dev_payload->ServerIp);
	free(set_dev_payload->Account);
	free(set_dev_payload->Pwd);
	free(set_dev_payload->EZPROIP);
	free(set_dev_payload);

	if(i < 0)
	{
		ResponseNotAccept(sock_obj);
		return;
	}
	ResponseAccept(sock_obj);

	ClientService_Restart();
}
#endif
void UpdateDeviceLocation(SOCKET_DATA* sock_obj)
{
	int i = 0;
	SETTING_LOCATION* loc_payload = (SETTING_LOCATION*)malloc(sizeof(SETTING_LOCATION));
	memset(loc_payload,0,sizeof(SETTING_LOCATION));
	int tmpIndex = 0, nextIndex;
	char target[2048], key[1024], value[1024];

	char* FullURL=(char*)malloc(sizeof(char)*(sock_obj->length)+1);
	memcpy(FullURL,sock_obj->payload,sock_obj->length);
	FullURL[sock_obj->length]='\0';

	while(*(FullURL + tmpIndex + 1) != '\0')
	{
		memset(target, 0, 2048);
		memset(key, 0, 1024);
		memset(value, 0, 1024);
		
		nextIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
		strncpy(target, sock_obj->payload + tmpIndex + 1, nextIndex - tmpIndex);
		sscanf(target, "%s %s", key, value);
		ConvertToLowerCase(key, strlen(key));

		if(strncmp(key, "branchid", 8) == 0)
		{
			loc_payload->BranchId = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "longitude", 9) == 0)
		{
			loc_payload->Longitude = atof(value);
		}
		else if(strncmp(key, "latitude", 8) == 0)
		{
			loc_payload->Latitude = atof(value);
		} 
		
		tmpIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
	}

#ifdef DEBUG
	printf("%d,%lf,%lf\n", loc_payload->BranchId
						  , loc_payload->Longitude
						  , loc_payload->Latitude);
#endif

	i = SetDeviceLocation(loc_payload);

	free(loc_payload);

	if(i < 0)
	{
		ResponseNotAccept(sock_obj);
		free(FullURL);
		return;
	}
	ResponseAccept(sock_obj);
	free(FullURL);
}

void UpdateThresholdSetting(SOCKET_DATA* sock_obj)
{
	char *tok, **tmp_str;
	int i, Id, Func, Enable, tmpIndex = 0, nextIndex;
	float TargetValue;
	char target[2048], key[1024], value[1024];

	while(*(sock_obj->payload + tmpIndex + 1) != '\0')
	{
		memset(target, 0, 2048);
		memset(key, 0, 1024);
		memset(value, 0, 1024);
		
		nextIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
		strncpy(target, sock_obj->payload + tmpIndex + 1, nextIndex - tmpIndex);
		
		sscanf(target, "%s %s", key, value);
		ConvertToLowerCase(key, strlen(key));

		if(strncmp(key, "id", 2) == 0)
		{
			Id = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "func", 4) == 0)
		{
			Func = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "enable", 6) == 0)
		{
			Enable = strtol(value, NULL, 0);
		}
		else if(strncmp(key, "value", 5) == 0)
		{
			TargetValue = atof(value);
		}
		
		tmpIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
	}

#ifdef DEBUG
	printf("%d,%d,%d,%f\n", Id
						  , Func
						  , Enable
						  , TargetValue);
#endif

	i = UpdateThreshold(Id, (char)Enable, Func, TargetValue);

	if(i < 0)
	{
		ResponseNotAccept(sock_obj);
		return;
	}
	ResponseAccept(sock_obj);
}

void RestartService(SOCKET_DATA* sock_obj)
{
	ClientService_Restart();
	ResponseAccept(sock_obj);
}

void ResponseCheckLog(SOCKET_DATA* sock_obj)
{
	int i = 0, Id, tmpIndex = 0, nextIndex;
	char target[2048], key[1024], value[1024];

	while(*(sock_obj->payload + tmpIndex + 1) != '\0')
	{
		memset(target, 0, 2048);
		memset(key, 0, 1024);
		memset(value, 0, 1024);
		
		nextIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
		strncpy(target, sock_obj->payload + tmpIndex + 1, nextIndex - tmpIndex);
		sscanf(target, "%s %s", key, value);
		ConvertToLowerCase(key, strlen(key));

		if(strncmp(key, "logid", 5) == 0)
		{
			Id = strtol(value, NULL, 0);
		}
		
		tmpIndex = strcspn(sock_obj->payload + tmpIndex + 1, "\n") + tmpIndex + 1;
	}

#ifdef DEBUG
	printf("Get Id=%d\n", Id);
#endif

	i = CheckedLog(Id);

	if(i < 0)
	{
		ResponseNotAccept(sock_obj);
		return;
	}
	ResponseAccept(sock_obj);
}
void GetSettingAPIRequest(SOCKET_DATA* sock_obj, char* requestSTR)
{
	if(strncmp(requestSTR, "GetDeviceSetting", 16) == 0)
	{
		ResponseOKPackage(sock_obj, DeviceSetting);
	}
	else if(strncmp(requestSTR, "GetDeviceLocation", 17) == 0)
	{
		ResponseOKPackage(sock_obj, DeviceLocation);
	}
	else if(strncmp(requestSTR, "GetThresholdSetting", 19) == 0)
	{
		ResponseOKPackage(sock_obj, ThresholdSetting);
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}
}

void GetLogAPIRequest(SOCKET_DATA* sock_obj, char* requestSTR)
{
	char* tok, *tok2;

	if(strncmp(requestSTR, "GetDataLog", 10) == 0)
	{
		tok = strtok(requestSTR, "?");
		tok = strtok(NULL, "?");
		tok2 = strtok(tok, "=");
		tok2 = strtok(NULL, "=");
		ResponseLogData(sock_obj, atoi(tok2));
	}
	else if(strncmp(requestSTR, "GetUnreadLogCount", 17) == 0)
	{
		ResponseUnreadLogCount(sock_obj);
	}
	else if(strncmp(requestSTR, "GetUnreadLogBaner", 17) == 0)
	{
		ResponseOKPackage(sock_obj, UnreadLog);	
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}
}


void GetDashboardAPIRequest(SOCKET_DATA* sock_obj, char* requestSTR)
{
	if(strncmp(requestSTR, "Get", 3) == 0)
	{
		ResponseOKPackage(sock_obj, DashboardData);
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}
}

void PostSettingAPI(SOCKET_DATA* sock_obj, char* requestSTR)
{

	if(strncmp(requestSTR, "SetDeviceSetting", 16) == 0)
	{
		UpdateDeviceSetting(sock_obj);
	}
	else if(strncmp(requestSTR, "SetDeviceLocation", 17) == 0)
	{
		UpdateDeviceLocation(sock_obj);
	}
	else if(strncmp(requestSTR, "SetThreshold", 12) == 0)
	{
		UpdateThresholdSetting(sock_obj);
	}
	else if(strncmp(requestSTR, "RestartService", 14) == 0)
	{
		RestartService(sock_obj);
	}
#ifdef iON
	if(strncmp(requestSTR, "iON_SetDeviceSetting", 20) == 0)
	{
		Update_iON_DeviceSetting(sock_obj);
	}
#endif
	else
	{
		ResponseNotSupport(sock_obj);
	}
}

void PutLogAPI(SOCKET_DATA* sock_obj, char* requestSTR)
{
	if(strncmp(requestSTR, "CheckLog", 8) == 0)
	{
		ResponseCheckLog(sock_obj);
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}
}


void ResponseFile(SOCKET_DATA* sock_obj, char* file_name, char* file_ext)
{
	char ch, *file_path, *file_content;
	long file_size;
	size_t result;

	file_path = (char*)malloc(sizeof(char) * (strlen(file_name) + 100));

	sprintf(file_path, WEBPAGE_PATH"%s", file_name);

	FILE *fp = fopen(file_path, "r");

	if(fp == NULL)
	{
		ResponseNotSupport(sock_obj);
		free(file_path);
		return;
	}
	
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	file_content = (char*)malloc(sizeof(char) * file_size + 1);
	result = fread(file_content, 1, file_size, fp);

	SA_SendPackages_Bytes(sock_obj, "HTTP/1.0 200 OK\r\nContent-Type: ", 31);

	if(strncmp(file_ext, "html", 3) == 0)	
	{
		SA_SendPackages_Bytes(sock_obj, "text/html\r\n\r\n", 13);
	}
	else if(strncmp(file_ext, "css", 3) == 0)
	{
		SA_SendPackages_Bytes(sock_obj, "text/css\r\n\r\n", 12);
	}
	else if(strncmp(file_ext, "js", 2) == 0)
	{
		SA_SendPackages_Bytes(sock_obj, "application/javascript\r\n\r\n", 26);
	}
	else if(strncmp(file_ext, "png", 3) == 0)
	{
		SA_SendPackages_Bytes(sock_obj, "image/png\r\n\r\n", 13);
	}

	SA_SendPackages_Bytes(sock_obj, file_content, file_size);

	SA_SendPackages(sock_obj, "\r\n");

	fclose(fp);
	free(file_path);
	free(file_content);
}

void GetRequestParameter(char* FullURL, char* APIPath, char* RequestURL,HEADER *header)
{
	int APIPathIndex,requestIndex;
	char method[255]={0};
	char requestSTR[1024]={0};

	sscanf(FullURL, "%s %s", method, requestSTR);
#ifndef DAEMON
	LogAgent_Write((char*)"Receive %s request, URL:%s", method, requestSTR);
#endif
	APIPathIndex = strcspn(requestSTR, "/");
	requestIndex = strcspn(&requestSTR[1], "/");

	strncpy(APIPath, &requestSTR[1], requestIndex - APIPathIndex);
	strcpy(RequestURL, (requestSTR + requestIndex + 2));
}

void WEB_GETRequest(SOCKET_DATA* sock_obj)
{
	int extIndex;
	char *ext;
	char APIPath[1024]={0};
	char requestURL[1024]={0};
	curr_header=NULL;

	char* FullURL=(char*)malloc(sizeof(char)*(sock_obj->length)+1);
	memcpy(FullURL,sock_obj->payload,sock_obj->length);
	FullURL[sock_obj->length]='\0';

	GetRequestParameter(FullURL, APIPath, requestURL,curr_header);

	free(FullURL);
	if(strncmp(APIPath, "DashboardAPI", 12) == 0)
	{
		GetDashboardAPIRequest(sock_obj, requestURL);
	}
	else if(strncmp(APIPath, "LogAPI", 6) == 0)
	{
		GetLogAPIRequest(sock_obj, requestURL);
	}
	else if(strncmp(APIPath, "SettingAPI", 10) == 0)
	{
		GetSettingAPIRequest(sock_obj, requestURL);
	}
	else
	{
		if(strlen(APIPath) < 2)
		{
			ResponseFile(sock_obj, (char*)"index.html", (char*)"html");
			SA_DestroySocketObject(sock_obj);
			return;
		}

		extIndex = strcspn(APIPath, ".");
		ext = &(*(APIPath + extIndex + 1));

		if(strlen(ext) > 0)
		{
			ResponseFile(sock_obj, APIPath, ext);
		}
		else
		{
			ResponseNotSupport(sock_obj);
		}
	}
	SA_DestroySocketObject(sock_obj);
}

void WEB_POSTRequest(SOCKET_DATA* sock_obj)
{
	char APIPath[1024]={0};
	char requestURL[1024]={0};

	curr_header=NULL;

	char* FullURL=(char*)malloc(sizeof(char)*(sock_obj->length)+1);
	memcpy(FullURL,sock_obj->payload,sock_obj->length);
	FullURL[sock_obj->length]='\0';

	GetRequestParameter(FullURL, APIPath, requestURL,curr_header);
	free(FullURL);

	if(strncmp(APIPath, "SettingAPI", 10) == 0)
	{
		PostSettingAPI(sock_obj, requestURL);
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}

	

	SA_DestroySocketObject(sock_obj);
}

void WEB_PUTRequest(SOCKET_DATA* sock_obj)
{
	char APIPath[1024]={0};
	char requestURL[1024]={0};

	curr_header=NULL;

	char* FullURL=(char*)malloc(sizeof(char)*(sock_obj->length)+1);
	memcpy(FullURL,sock_obj->payload,sock_obj->length);
	FullURL[sock_obj->length]='\0';

	GetRequestParameter(FullURL, APIPath, requestURL,curr_header);
	free(FullURL);

	if(strncmp(APIPath, "LogAPI", 6) == 0)
	{
		PutLogAPI(sock_obj, requestURL);
	}
	else
	{
		ResponseNotSupport(sock_obj);
	}

	SA_DestroySocketObject(sock_obj);
}
