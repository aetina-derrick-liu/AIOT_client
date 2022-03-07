#include <stdio.h>
#include "Chamber.hpp"
#ifdef ICHAMBER
#include "ClientService.hpp"
#include "DeviceAgent.hpp"
#include "screenshot.hpp"
extern "C"
{
	#include "chamberctl.h"
}

static int ChamberPortNumber;
extern char GatewayAddress[MAX_SERVICE_STR_SIZE],
		DeviceName[MAX_DEVICENAME_SIZE],
		PWD[MAX_PWD_SIZE];
#endif



void StartChamberAgent()
{
#ifdef ICHAMBER	
	printf("iChamber is enabled.\n");
	if(ChamberPortNumber == 0)
	{
		printf("Chamber prot number is 0, disable chamber control function, enable screenshot\n");
		StartScreenshotService(DeviceName);
	}
	else
	{
		if(LoadChamberLibraries())
		{
			printf("Load dependencies libraries fail.\n");
		}
		else
		{
			printf("Load dependencies libraries successful.\n");
			printf("Start iChamber service\n");
			int retcode = StartChamberController(3);
			if(retcode != 0)
			{
				printf("Start iChamber service fail\n");
			}
		}
	}
#endif
}

void StopChamberAgent()
{
#ifdef ICHAMBER
	printf("Stop iChamber service\n");
	StopChamberController();
	StopScreenshotService();
#endif
}