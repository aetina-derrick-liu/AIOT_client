#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CMDParser.h"
#include "MEMInfo.h"
#ifdef WIN32
	#include "libcpu.h"
	extern SPD_INFO spd;
#endif

#ifndef WIN32

#ifdef __aarch64__
UINT GetMEMCapacity()
{
	char* ret;
	UINT retValue;

	ret = MEMAllocting(__func__);	

	memset(ret, 0, MAX_CMD_STR_LEN);

	GetCommandResult(ret, "cat /proc/meminfo | grep MemTotal", " :", 1);

	retValue = atoi(ret);


	free(ret);	

	return retValue;

}
USHORT GetNumberOfMEM()
{
	char* ret;
	USHORT retValue;

	ret = MEMAllocting(__func__);	
		
	GetCommandResult(ret, "lshw -class memory | grep bank -c", NULL, 0);

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}
#else
UINT GetMEMCapacity()
{
	char* ret;
	UINT retValue;

	ret = MEMAllocting(__func__);	
	
	GetCommandResult(ret, "dmidecode -t 19 | grep Range", " :", 2);

	retValue = atoi(ret);

	memset(ret, 0, MAX_CMD_STR_LEN);

	GetCommandResult(ret, "dmidecode -t 19 | grep Range", " :", 3);


	if(strcmp(ret, "GB") == 0)
	{
		retValue *= 1024 * 1024;
	}
	else if(strcmp(ret, "MB") == 0)
	{
		retValue *= 1024;
	}

	free(ret);	

	return retValue;

}

USHORT GetNumberOfMEM()
{
	char* ret;
	USHORT retValue;

	ret = MEMAllocting(__func__);	
		
	GetCommandResult(ret, "dmidecode -t 17 | grep Size -c", NULL, 0);

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}
#endif
char* GetMEMSingleType(USHORT index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Type: | tail -n +%u", index +1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);

	return MEMReallocting(ret);
}
char* GetMEMSingleSN(USHORT index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Serial | tail -n +%u", index +1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);

	return MEMReallocting(ret);
}
char* GetMEMSingleManufacturer(USHORT index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Manu | tail -n +%u", index +1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);

	return MEMReallocting(ret);
}

char* GetMEMSingleLocation(USHORT index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Loc | tail -n +%u", (index*2)+1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);

	return MEMReallocting(ret);
}

UINT GetMEMSingleCapacity(USHORT index)
{
	char* ret, *cmd;
	UINT retValue;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Size | tail -n +%u", index + 1);

	GetCommandResult(ret, cmd, ":M", 1);

	retValue = atoi(ret);

	free(ret);	
	free(cmd);	

	retValue *= 1024;

	return retValue;
}

USHORT GetMEMSingleFrequency(USHORT index)
{
	char* ret, *cmd;
	USHORT retValue;
	
	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
	
	sprintf(cmd, "dmidecode -t 17| grep \"Clock Speed\" | tail -n +%u", index + 1);

	GetCommandResult(ret, cmd, ":M", 1);

	retValue = atoi(ret);

	free(ret);
	free(cmd);	

	return retValue;
}

float GetMEMSingleVoltage(USHORT index)
{
	char* ret, *cmd;
	float retValue;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17| grep \"Configured Voltage\" | tail -n +%u", index + 1);

	GetCommandResult(ret, cmd, ":V", 2);

	retValue = atof(ret);

	free(ret);
	free(cmd);

	return retValue;
}
char* GetMEMSinglePN(USHORT index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "dmidecode -t 17 | grep Part | tail -n +%u", index +1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);

	return MEMReallocting(ret);
}
#endif

MEM_INFO* lib_SysInfo_GetMEMInfo()
{
	MEM_INFO* ret = (MEM_INFO*)malloc(sizeof(MEM_INFO));
#ifdef WIN32
	char** tmp = GetWMIData("ROOT\\CIMV2",
							"SELECT * FROM Win32_MemoryArray",
							0,
							1,
							"EndingAddress");
	ret->Capacity = atoi(*(tmp+0)) + 1;
	free(*(tmp+0));
	free(tmp);

    memset(&spd, 0, sizeof(spd));
	if(!init_lib())	return 0;
	get_spd_info(&spd);
	ret->NumberOfMEM =spd.iSlots;
	PRODUCT_DATA *product;
	FEATURE_DATA *feature;
#else
	ret->Capacity = GetMEMCapacity();
	ret->NumberOfMEM = GetNumberOfMEM();
#endif
	MEMSLOT_INFO* prev_node = NULL;
#ifdef WIN32
	if(spd.iSlots > 0)
	{
		for(int i=0; i < spd.iSlots; i++)
    	{
#else
	if(ret->NumberOfMEM!=0)
	{

		for(USHORT i = 0; i < ret->NumberOfMEM; i++)
		{
#endif
			MEMSLOT_INFO* current_node = (MEMSLOT_INFO*)malloc(sizeof(MEMSLOT_INFO));
			current_node->next_node = NULL;
			if(prev_node != NULL)
			{
				prev_node->next_node = (void*)current_node;
			}
			else
			{
				current_node->prev_node = NULL;
			}
			current_node->Index = i;
#ifdef WIN32
			product = &spd.product[i];
			feature = &spd.feature[i];
			current_node-> Type = (char*)malloc(sizeof(char)*strlen(product->sDRAMType)+1);
			memset(current_node-> Type,0,sizeof(current_node-> Type));
			strcpy(current_node-> Type,product->sDRAMType);
			
			current_node-> DIMMType = (char*)malloc(sizeof(char)*strlen(product->sDIMMType)+1);
			memset(current_node-> DIMMType,0,sizeof(current_node-> DIMMType));
			strcpy(current_node-> DIMMType,product->sDIMMType);

			current_node-> Manufacturer = (char*)malloc(sizeof(char)*strlen(product->sMFG)+1);
			memset(current_node-> Manufacturer,0,sizeof(current_node-> Manufacturer));
			strcpy(current_node-> Manufacturer,product->sMFG);

//			current_node-> Location =  i+1;
			current_node-> OPTemp = (char*)malloc(sizeof(char)*strlen(product->sOperTemp)+1);
			memset(current_node-> OPTemp,0,sizeof(current_node-> OPTemp));
			strcpy(current_node-> OPTemp,product->sOperTemp);

			current_node-> IC_Cfg = (char*)malloc(sizeof(char)*strlen(product->sICConfiguration)+1);
			memset(current_node-> IC_Cfg,0,sizeof(current_node-> IC_Cfg));
			strcpy(current_node-> IC_Cfg,product->sICConfiguration);

			current_node-> IC_Brand = (char*)malloc(sizeof(char)*strlen(product->sICBrand)+1);
			memset(current_node-> IC_Brand,0,sizeof(current_node-> IC_Brand));
			strcpy(current_node-> IC_Brand,product->sICBrand);

			current_node-> Thermal = (char*)malloc(sizeof(char)*4);
			memset(current_node-> Thermal,0,4);
			if(product->bTSensor)
			{
				strncpy(current_node-> Thermal,"YES",3);
			}
			else
			{
				strncpy(current_node-> Thermal,"N/A",3);
			}
			current_node-> CAS_Ltc = product->fCASLatency;
			current_node-> Capacity = (product->iDRAMSize >= 1024) ? product->iDRAMSize / 1024 : product->iDRAMSize;
			
			current_node-> SN = (char*)malloc(sizeof(char)*strlen(product->sSN)+1);
			memset(current_node-> SN,0,sizeof(current_node-> SN));
			strcpy(current_node-> SN,product->sSN);

			current_node-> PN = (char*)malloc(sizeof(char)*strlen(product->sPN)+1);
			memset(current_node-> PN,0,sizeof(current_node-> PN));
			strcpy(current_node-> PN,product->sPN);

			current_node-> Date = (char*)malloc(sizeof(char)*strlen(product->sMFGDate)+1);
			memset(current_node-> Date,0,sizeof(current_node-> Date));
			strcpy(current_node-> Date,product->sMFGDate);

			current_node-> Rate = product->iDataRateMTs;
			if(product->bIsInno)
			{
				current_node->feature = (MEMSLOT_FEATURE*)malloc(sizeof(MEMSLOT_FEATURE));

				current_node->feature->sICGrade = (char*)malloc(sizeof(char)*strlen(feature->sICGrade)+1);
				memset(current_node->feature->sICGrade,0,sizeof(current_node->feature->sICGrade));
				strcpy(current_node->feature->sICGrade,feature->sICGrade);

				current_node->feature->AntiSul = (feature->iAntiS == 1) ?  1 : 2 ?  1 : 0;
				if(feature->b45GoldenFinger)
				{
					current_node->feature-> b45GoldenFing = 1;
				}
				else
				{
					current_node->feature-> b30GoldenFing = (feature->b30GoldenFinger) ? 1 : 0;
				}
				current_node->feature-> bWP = (feature->bWP) ? 1 : 0;
			}
#else
			current_node->Type = GetMEMSingleType(i);
			current_node->Manufacturer = GetMEMSingleManufacturer(i);
			current_node->Location = GetMEMSingleLocation(i);
			current_node->Capacity = GetMEMSingleCapacity(i);
			current_node->SN = GetMEMSingleSN(i);
			current_node->PN = GetMEMSinglePN(i);
			current_node->Rate = GetMEMSingleFrequency(i);
#endif
			if(prev_node != NULL)
			{
				current_node->prev_node = (void*)prev_node;
			}
			else
			{
				ret->MEMSlot = current_node;
			}
			prev_node = current_node; 
		}
	}
	else
	{
		ret->MEMSlot=0;
	}
	prev_node = ret->MEMSlot;
	return ret;
}
UINT lib_SysInfo_GetMEMLess()
{
	char* ret;
	UINT retValue;

#ifdef WIN32
	HW_INFO  hw;
	memset(&hw,  0, sizeof(hw)); 
	if(!init_lib())
        return 0;
	get_hw_info(&hw);
	retValue = hw.iMEMUsage;
#else
	ret = MEMAllocting(__func__);

	GetCommandResult(ret, "grep '^MemAvailable' /proc/meminfo", " :", 1);

	retValue = atoi(ret);

	free(ret);	
#endif
	return retValue;
}

float lib_SysInfo_GetMEMTemp()
{
	char* ret;
	float retValue = 0;

#ifdef WIN32
	HW_INFO  hw;
	memset(&hw,  0, sizeof(hw)); 
	if(!init_lib())
        return 0;
	get_hw_info(&hw);
	HW_TEMP_DATA *hw_temp = &hw.HWTemp;
	if(hw_temp->fTa != -999)
		retValue = hw_temp->fTa;
#else
/*
	ret = MEMAllocting(__func__);

	GetCommandResult(ret, "grep '^MemAvailable' /proc/meminfo", " :", 1);

	retValue = atoi(ret);

	free(ret);	
*/
#endif
	return retValue;
}