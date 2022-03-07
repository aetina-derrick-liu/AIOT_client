#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "libiCAPClient.h"
#include "DeviceAgent.h"
#include "SocketAgent.h"

int lastErrorCode = 0;

int iCAP_Connect()
{
	int ret;
	if(serverRunning == 1)
	{
		return 0;
	}
#ifdef WIN32
	CreateThread(NULL, 0, SA_Start, NULL, 0, 0);
	Sleep(1000);
	if(serverRunning != 1)
	{
		return 1;
	}
#else
	ret = SA_Start(2889);
	if(ret != 0)
	{
		return 1;
	}
#endif
	
	return 0;
}

int iCAP_Disconnect()
{
	SA_Stop();
	return 0;
}

int iCAP_GetClientStatus()
{
	int ret;
	ret = SA_CheckStatus();
	return ret;
}

char* iCAP_GetLastErrorMsg()
{

}

int iCAP_SetResponseInterval(unsigned int ms)
{
	int ret;
	ret = SA_SetInterval(ms);
	return ret;
}

int iCAP_AddExternalSensor(char* name, char* unit, unsigned char type, GetValueFunc func)
{
	int ret;
	ret = DA_AddExtSensorToList(name, func);
	if(ret == 0)
	{
		ret = SA_AddExtSensor(name, unit, type);
		if(ret != 0)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int iCAP_AddStaticInfo(char* name,char* info)
{
	int ret;
	ret = SA_AddStaticInfo(name,info);
	if(ret != 0)
	{
		return 1;
	}
	return 0;
}

int iCAP_RemoveExternalSensor(char* name)
{
	int ret;
	ret = DA_RemoveExtSensor(name);
	if(ret == 0)
	{
		ret = SA_RemoveExtSensor(name);
		if(ret != 0)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int iCAP_AddRemoteDevice(char* name, char* unit, unsigned char type, SendRemoteFunc func)
{
	int ret;
	ret = DA_AddRemoteDeviceToList(name, func);
	if(ret == 0)
	{
		ret = SA_AddRemoteDev(name, unit, type);
		if(ret != 0)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int iCAP_RemoveRemoteDevice(char* name)
{
	int ret;
	ret = DA_RemoveRemoteDevice(name);
	if(ret == 0)
	{
		ret = SA_RemoveRemoteDev(name);
		if(ret != 0)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int iCAP_EventTrigger()
{
	int ret;
	ret = SA_EventTrigger();
	if(ret != 0)
	{
		return 1;
	}
	return 0;
}

int iCAP_SendLogout()
{
	int ret;
	ret = SA_SendLogout();
	if(ret != 0)
	{
		return 1;
	}
	return 0;
}
