#include <cstring>
#include "ClientService.hpp"
#include "LogAgent.hpp"
#include "DBInterface.hpp"
#include "WebCommand.hpp"
#include "Base64.hpp"
#include "DeviceAgent.hpp"
#include "MQTTAgent.hpp"
#include "DataBuilder.hpp"
#ifdef ICHAMBER
	#include "Chamber.hpp"
#endif


extern char GatewayAddress[MAX_SERVICE_STR_SIZE],
		DeviceName[MAX_DEVICENAME_SIZE],
		PWD[MAX_PWD_SIZE];

extern char WebAddress[MAX_SERVICE_STR_SIZE];

int SetServiceParameters()
{
	extern double Longitude;
	extern double Latitude;
	extern unsigned long Interval;
	
	char *addr;

	addr = GetServiceIPAddress();

	GetNumberOfDeviceLocation();

	Longitude = GetDeviceLongitude(1);
	Latitude = GetDeviceLatitude(1);

	Interval = GetDeviceUploadInterval() * 1000;

	memset(GatewayAddress, 0, MAX_SERVICE_STR_SIZE);
	memset(WebAddress, 0, MAX_SERVICE_STR_SIZE);

	sprintf(GatewayAddress, "tcp://%s:1883", addr);
	sprintf(WebAddress, "http://%s", addr);

	free(addr);

	return 0;
}

int ClientService_Initialize()
{
	int retcode = 0;

	GetSysInfo();

// 	if(CheckInnodisk() == 0)
// 	{
// #ifdef iON
// 		LogAgent_Write((char*)"There doesnot exists any Ion storage.");
// #else
// 		LogAgent_Write((char*)"There doesnot exists any Innodisk devices.");
// #endif
// 		return -99;
// 	}
	retcode = CreateDB();
	if(retcode != 0)
	{
		LogAgent_Write((char*)"Failed to create client service DB.");
		return -1;
	}
	CheckDataGroup();

	CheckData();

	GetLocalStorageInfo();

	DeviceAgent_StartOffline();

	return 0;
}

int ClientService_Start()
{
	char deviceThumbprint_encode[MAX_SETTING_STR_SIZE];
	char *deviceThumbprint;
	int retcode = 0;

	retcode = SetServiceParameters();

	if(retcode < 0)
	{
		LogAgent_Write((char*)"Failed to set service setting.");
		return -3;
	}
	else
	{
		LogAgent_Write((char*)"Gateway address:%s", GatewayAddress);
		LogAgent_Write((char*)"Web service address:%s", WebAddress);
	}

	deviceThumbprint = GetDeviceThumbprint();

	base64_encode((unsigned char*)deviceThumbprint,
				  (unsigned char*)deviceThumbprint_encode,
				  strlen(deviceThumbprint),
				  0);

	retcode = WebCommand_DeviceAuth(deviceThumbprint_encode, DeviceName, PWD);

	free(deviceThumbprint);

	if(retcode < 0)
	{
		LogAgent_Write((char*)"Authentication failed.");
		WriteStatus(3);
		return -4;
	}

	if(strlen(DeviceName) < 1)
	{
		LogAgent_Write((char*)"Authentication failed, number of device has over limitation.");
		return -5;
	}

	SetDeviceName(DeviceName);

	if(MQTTAgent_Start() == 0)
	{

		LogAgent_Write((char*)"Connect to iCAP service gateway successfully.");

		if(DeviceAgent_Start())
		{
			LogAgent_Write((char*)"Start iCAP client service agent successfully.");
#ifdef ICHAMBER
			StartChamberAgent();
#endif
		}
		else
		{
			LogAgent_Write((char*)"Failed to start iCAP client service agent.");
			MQTTAgent_Stop();
			return -7;
		}
	}
	else
	{
#ifdef iON
		LogAgent_Write((char*)"Failed connection to EZ_SSD_SOS service gateway.");
#else
		LogAgent_Write((char*)"Failed connection to iCAP service gateway.");
#endif
		WriteStatus(1);
		return -6;
	}
	return 0;
}

void ClientService_Stop()
{
#ifdef ICHAMBER
	StopChamberAgent();
#endif
	SA_Stop();
	DeviceAgent_Stop();
	if(GetStatus() == 0)
	{
		MQTTAgent_Stop();
	}
#ifdef iON
		LogAgent_Write((char*)"EZ_SSD_SOS service agent has been stop.");
#else
		LogAgent_Write((char*)"iCAP client service agent has been stop.");
#endif
	WriteStatus(4);
	FreeSysInfo();
}

void ClientService_Restart()
{
	DeviceAgent_Stop();
	if(GetStatus() == 0)
	{
		MQTTAgent_Stop();
	}
	else
	{
		ClientService_Start();
	}
#ifdef iON
	LogAgent_Write((char*)"EZ_SSD_SOS service agent has been restart.");
#else
	LogAgent_Write((char*)"iCAP client service agent has been restart.");
#endif
}
