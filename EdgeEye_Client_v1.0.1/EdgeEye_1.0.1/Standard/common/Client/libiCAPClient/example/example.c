#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#endif

#include "libiCAPClient.h"

char onExit = 0;

#ifdef WIN32
BOOL WINAPI gotExitCmd(DWORD dwType)
{
    switch(dwType) {
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			onExit = 1;
			break;
    }
    return TRUE;
}
#else
void gotExitCmd(int sig)
{
	onExit = 1;
}

void gotPAUSECmd(int sig)
{
	printf("Send event trigger to service\n");
	iCAP_EventTrigger();
}
#endif


double GetSensorValue(void)
{
	return 25.5;
}

int main(int argc, char** argv)
{
	int ret;
	char c;

#ifdef WIN32
	if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)gotExitCmd,TRUE))
	{
		printf("Fail to listen Ctrl-C signal!\n");
	}
#else
	(void)signal(SIGINT, gotExitCmd);
	(void)signal(SIGTSTP, gotPAUSECmd);
#endif
	if(iCAP_Connect() == 0)
	{
		printf("Connect to device service successfully\n");
		
		ret = iCAP_GetClientStatus();
		printf("Device servcie status :%d\n", ret);

		iCAP_AddExternalSensor("Engine Speed", "rpm", 0, GetSensorValue);
		iCAP_AddExternalSensor("Tachograph Vehicle Speed", "km/h", 0, GetSensorValue);
		iCAP_AddExternalSensor("Engine Coolant Temperature", "'C", 0, GetSensorValue);
		iCAP_AddExternalSensor("Engine Total Fuel Used", "kg", 0, GetSensorValue);
		iCAP_AddExternalSensor("Boots Pressure", "kpg", 0, GetSensorValue);
		iCAP_AddExternalSensor("Accelerator Pedal Position", "%", 0, GetSensorValue);

		while(onExit == 0)
		{
			usleep(10000L);
		}

		iCAP_RemoveExternalSensor("Engine Speed");
		iCAP_RemoveExternalSensor("Tachograph Vehicle Speed");
		iCAP_RemoveExternalSensor("Engine Coolant Temperature");
		iCAP_RemoveExternalSensor("Engine Total Fuel Used");
		iCAP_RemoveExternalSensor("Boots Pressure");
		iCAP_RemoveExternalSensor("Accelerator Pedal Position");

		iCAP_Disconnect();
	}
	else
	{
		printf("Connect to device service fail\n");
	}
	return 0;
}
