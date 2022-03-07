#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JsonParser.h"
#include "DeviceAgent.h"
#include "SocketAgent.h"

ExtSensorList* ExtSensorListHead = NULL;
RemoteDeviceList* RemoteDeviceListHead = NULL;

int DA_AddExtSensorToList(char* name, GetValueFunc func)
{
	ExtSensorList* current_node = ExtSensorListHead;
	ExtSensorList* add_node = (ExtSensorList*)malloc(sizeof(ExtSensorList));
	if(add_node == NULL)
	{
		return -1;
	}
	add_node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));
	if(add_node->name == NULL)
	{
		return -2;
	}
	strcpy(add_node->name, name);
	add_node->func = func;
	add_node->prev_node = NULL;
	add_node->next_node = NULL;
	if(current_node == NULL)
	{
		ExtSensorListHead = add_node;
	}
	else
	{
		while(current_node->next_node != NULL)
		{
			current_node = (ExtSensorList*)current_node->next_node;
		}
		add_node->prev_node = (void*)current_node;
		current_node->next_node = (void*)add_node;
	}
	return 0;
}

int DA_RemoveExtSensor(char* name)
{
	ExtSensorList* current_node = ExtSensorListHead;
	ExtSensorList* prev_node, *next_node, *del_node;
	char modified = 0;
	while(current_node != NULL)
	{
		if(strcmp(current_node->name, name) == 0)
		{
			modified = 1;
			del_node = current_node;
			if(current_node->prev_node != NULL && current_node->next_node != NULL)
			{
				prev_node = (ExtSensorList*)del_node->prev_node;
				next_node = (ExtSensorList*)del_node->next_node;
				prev_node->next_node = (void*)next_node;
				next_node->prev_node = (void*)prev_node;
			}
			else if(current_node->prev_node == NULL && current_node->next_node != NULL)
			{
				next_node = (ExtSensorList*)del_node->next_node;
				next_node->prev_node = NULL;
				ExtSensorListHead = next_node;
			}
			else if(current_node->prev_node != NULL && current_node->next_node == NULL)
			{
				prev_node = (ExtSensorList*)del_node->prev_node;
				prev_node->next_node = NULL;
			}
			else
			{
				ExtSensorListHead = NULL;
			}
			free(del_node->name);
			del_node->func = NULL;
			free(del_node);
			break;
		}
		current_node = (ExtSensorList*)current_node->next_node;
	}
	if(modified)
	{
		return 0;
	}
	return -1;
}

double DA_GetExtSensorValue(const char* name)
{
	ExtSensorList* current_node = ExtSensorListHead;
	double ret = 0;
	while(current_node != NULL)
	{
		if(strcmp(current_node->name, name) == 0)
		{
			ret = current_node->func();
			break;
		}
		current_node = (ExtSensorList*)current_node->next_node;
	}
	return ret;
}

int DA_AddRemoteDeviceToList(char* name, SendRemoteFunc func)
{
	RemoteDeviceList* current_node = RemoteDeviceListHead;
	RemoteDeviceList* add_node = (RemoteDeviceList*)malloc(sizeof(ExtSensorList));
	if(add_node == NULL)
	{
		return -1;
	}
	add_node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));
	if(add_node->name == NULL)
	{
		return -2;
	}
	strcpy(add_node->name, name);
	add_node->func = func;
	add_node->prev_node = NULL;
	add_node->next_node = NULL;
	if(current_node == NULL)
	{
		RemoteDeviceListHead = add_node;
	}
	else
	{
		while(current_node->next_node != NULL)
		{
			current_node = (RemoteDeviceList*)current_node->next_node;
		}
		add_node->prev_node = (void*)current_node;
		current_node->next_node = (void*)add_node;
	}
	return 0;
}

int DA_RemoveRemoteDevice(char* name)
{
	RemoteDeviceList* current_node = RemoteDeviceListHead;
	RemoteDeviceList* prev_node, *next_node, *del_node;
	char modified = 0;
	while(current_node != NULL)
	{
		if(strcmp(current_node->name, name) == 0)
		{
			modified = 1;
			del_node = current_node;
			if(current_node->prev_node != NULL && current_node->next_node != NULL)
			{
				prev_node = (RemoteDeviceList*)del_node->prev_node;
				next_node = (RemoteDeviceList*)del_node->next_node;
				prev_node->next_node = (void*)next_node;
				next_node->prev_node = (void*)prev_node;
			}
			else if(current_node->prev_node == NULL && current_node->next_node != NULL)
			{
				next_node = (RemoteDeviceList*)del_node->next_node;
				next_node->prev_node = NULL;
				RemoteDeviceListHead = next_node;
			}
			else if(current_node->prev_node != NULL && current_node->next_node == NULL)
			{
				prev_node = (RemoteDeviceList*)del_node->prev_node;
				prev_node->next_node = NULL;
			}
			else
			{
				RemoteDeviceListHead = NULL;
			}
			free(del_node->name);
			del_node->func = NULL;
			free(del_node);
			break;
		}
		current_node = (RemoteDeviceList*)current_node->next_node;
	}
	if(modified)
	{
		return 0;
	}
	return -1;
}

int DA_SendRemoteDeviceCmd(const char* name, void* value)
{
	RemoteDeviceList* current_node = RemoteDeviceListHead;
	int ret = 0;
	while(current_node != NULL)
	{
		if(strcmp(current_node->name, name) == 0)
		{
			ret = current_node->func(value);
			break;
		}
		current_node = (RemoteDeviceList*)current_node->next_node;
	}
	return ret;
}

#ifdef WIN32
void DA_GetServiceCmd(SOCKET client_sock, char* payload)
#else
void* DA_GetServiceCmd(void* sockdata)
#endif
{
#ifndef WIN32
	SOCKET_DATA* ptr = (SOCKET_DATA*)sockdata;
#endif
	struct json_object* jobj, *cmd, *name, *remoteCmd;

#ifdef WIN32
	jobj = json_tokener_parse(payload);
#else
	jobj = json_tokener_parse(ptr->payload);
#endif
	if(is_error(jobj))
	{
		printf("Get invalid message, drop.\n");
	}
	else
	{
		cmd = get_json_object(jobj, "Cmd");
		if(cmd)
		{
			if(strcmp(json_object_get_string(cmd), "GetExt") == 0)
			{
				printf("Get external sensor value command from service.\n");
				double retValue = 0;
				name = get_json_object(jobj, "Name");
				retValue = DA_GetExtSensorValue(json_object_get_string(name));
#ifdef WIN32
				SA_ResponseExtSensorValue(client_sock, json_object_get_string(name), retValue);
#else
				SA_ResponseExtSensorValue(ptr->sockfd, json_object_get_string(name), retValue);
#endif
			}
			if(strcmp(json_object_get_string(cmd), "Remote") == 0)
			{
				printf("Get remote command from service.\n");
				int ret = 0;
				name = get_json_object(jobj, "Name");
				remoteCmd = get_json_object(jobj, "Value");
				ret = DA_SendRemoteDeviceCmd(
						json_object_get_string(name),
						(void*)json_object_get_string(remoteCmd));
				if(ret == 0)
				{
#ifdef WIN32
					SA_ResponseRemoteOKCmd(client_sock, json_object_get_string(name));
#else
					SA_ResponseRemoteOKCmd(ptr->sockfd, json_object_get_string(name));
#endif
				}
				else
				{
#ifdef WIN32
					SA_ResponseRemoteFailCmd(client_sock, json_object_get_string(name));
#else
					SA_ResponseRemoteFailCmd(ptr->sockfd, json_object_get_string(name));
#endif
				}
			}
		}
	}
	json_object_put(jobj);
}

