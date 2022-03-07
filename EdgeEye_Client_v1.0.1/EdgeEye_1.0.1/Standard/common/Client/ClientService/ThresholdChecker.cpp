#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DBInterface.hpp"
#include "ThresholdChecker.hpp"

#ifdef iON
extern "C"
{
	#include "EZPROAPI.h"
}
#endif

int CheckThreshold(int Func, float Threshold, float Value)
{
	switch(Func)
	{
		case 0:
			if(Value == Threshold)
			{
				return 1;
			}
			break;
		case 1:
			if(Value < Threshold)
			{
				return 1;
			}
			break;
		case 2:
			if(Value > Threshold)
			{
				return 1;
			}
			break;
		case 3:
			if(Value != Threshold)
			{
				return 1;
			}
			break;
	}
	return 0;
}

void CheckStorageThreshold(char* SN, int Temperature, int AvgEraseCount, int Lifespan)
{
	THRESHOLD_SETTING* threshold;
	char eventLogStr[1024];

	CheckEventLogCount();

	//Calculate temperature
	memset(eventLogStr, 0, 1024);
	threshold = GetThreshold(41);
	if(threshold->Enable == 1)
	{
		if(CheckThreshold(threshold->Func, threshold->Value, Temperature) == 1)
		{
			sprintf(eventLogStr, 
					"Storage %s temperature over threshold, value: %d celsius.",
					SN,
					Temperature);
			WriteEventLog(eventLogStr);
#ifdef iON
			SETTING_iON_DEVICE* ez;
			ez = Get_iON_EZPROParameters();
			char* ret=Webcommand_CreateEvent(ez,eventLogStr);
			if(!strncmp(ret,"AUTHFAILED",10))
			{
				WriteEventLog("EZRPO connection failed:wrong account or password.");
			}
			free(ret);
			free(ez->Account);
			free(ez->Pwd);
			free(ez->EZPROIP);
			free(ez);
#endif
		}
	}
	free(threshold);

	//Calculate lifespan
	memset(eventLogStr, 0, 1024);
	threshold = GetThreshold(45);
	if(threshold->Enable == 1)
	{
		if(Lifespan >= 0)
		{
			if(CheckThreshold(threshold->Func, threshold->Value, Lifespan) == 1)
			{
				sprintf(eventLogStr, 
						"Storage %s lifespan over threshold, value: %d days.",
						SN,
						Lifespan);
				WriteEventLog(eventLogStr);
#ifdef iON
			SETTING_iON_DEVICE* ez;
			ez = Get_iON_EZPROParameters();
			char* ret=Webcommand_CreateEvent(ez,eventLogStr);
			if(!strncmp(ret,"AUTHFAILED",10))
			{
				WriteEventLog("EZRPO connection failed:wrong account or password.");
			}
			free(ret);
			free(ez->Account);
			free(ez->Pwd);
			free(ez->EZPROIP);
			free(ez);
#endif
			}
		}
	}
	free(threshold);

	//Calculate avgEraseCount
	memset(eventLogStr, 0, 1024);
	threshold = GetThreshold(43);
	if(threshold->Enable == 1)
	{
		if(CheckThreshold(threshold->Func, threshold->Value, AvgEraseCount) == 1)
		{
			sprintf(eventLogStr, 
					"Storage %s average erase count over threshold, value: %d times.",
					SN,
					AvgEraseCount);
			WriteEventLog(eventLogStr);
#ifdef iON
			SETTING_iON_DEVICE* ez;
			ez = Get_iON_EZPROParameters();
			char* ret=Webcommand_CreateEvent(ez,eventLogStr);
			if(!strncmp(ret,"AUTHFAILED",10))
			{
				WriteEventLog("EZRPO connection failed:wrong account or password.");
			}
			free(ret);
			free(ez->Account);
			free(ez->Pwd);
			free(ez->EZPROIP);
			free(ez);
#endif
		}
	}
	free(threshold);
}
