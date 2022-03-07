#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CMDParser.h"
#include "MBInfo.h"

#ifndef WIN32
char* GetMBManufacturer()
{
	char* ret;

	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	ret = strcpy(ret, "Aetina Corporation.");
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +2", ":", 1);
#endif
	

	return MEMReallocting(ret);
}

char* GetMBProductNameTemp()
{
	char* ret;

	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	ret = strcpy(ret, "AIE-CN11");
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +2", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetMBProductName(char* MB_info)
{
	char* ret;
	char* item = "Product";
	char* result;
	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	result = GetJetsonResult(MB_info,item,11);
	
	ret = strcpy(ret,result);
	// printf("\nGetMBProductName : %s", ret);
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +3", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetMBProductNameCmd()
{
	char* ret;
	char* item = "Product";
	char* result;
	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	GetCommandResult(ret, "/opt/Jetson_status/Jetson_status MBInfo", NULL, 1);
	result = GetJetsonResult(ret,item,11);
	ret = strcpy(ret,result);
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +3", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetMBSerialNumberCmd()
{
	char* ret;
	char* item = "SN";
	char* result;
	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	GetCommandResult(ret, "/opt/Jetson_status/Jetson_status MBInfo", NULL, 1);
	result = GetJetsonResult(ret,item,6);
	ret = strcpy(ret,result);
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +3", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetMBSerialNumber(char* MB_Info)
{
	char* ret;
	char* item = "SN";
	char* result;
	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	result = GetJetsonResult(MB_Info,item,6);
	ret = strcpy(ret,result);
#else
	GetCommandResult(ret, "dmidecode | grep -A4 '^Base' | tail -n +5", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetBIOSManufacturer()
{
	char* ret;

	ret = MEMAllocting(__func__);	
#ifdef __aarch64__ //for jetson
	ret = strcpy(ret, "Aetina Corporation.");
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^Base' | tail -n +2", ":", 1);
#endif
	return MEMReallocting(ret);
}

char* GetBIOSVersion()
{
	char* ret;

	ret = MEMAllocting(__func__);	
#ifdef __aarch64__
	ret = strcpy(ret, "Aetina_0010101");
#else
	GetCommandResult(ret, "dmidecode | grep -A3 '^BIOS' | tail -n +3", ":", 1);
#endif

	return MEMReallocting(ret);
}
#endif

char* GetMB_Info()
{
	char* ret;
	ret = MEMAllocting(__func__);
	int count = 10;
	int i = 0;

	while (i <= count && (!strcmp(ret, "")))
	{
		i++;
		GetCommandResult(ret, "/opt/Jetson_status/Jetson_status MBInfo", NULL, 1);
	}
	return MEMReallocting(ret);
}

MB_INFO* lib_SysInfo_GetMBInfo()
{
	MB_INFO* ret = (MB_INFO*)malloc(sizeof(MB_INFO));
	ret->BIOSinfo = (BIOS_INFO*)malloc(sizeof(BIOS_INFO));
#ifdef WIN32
	char** tmp = GetWMIData("ROOT\\CIMV2",
						"SELECT * FROM Win32_BaseBoard",
						0,
						3,
						"Manufacturer",
						"Product",
						"SerialNumber");
	ret->Manufacturer = *(tmp+0);
	ret->ProductName = *(tmp+1);
	ret->SerialNumber = *(tmp+2);
	free(tmp);
	char** tmp2 = GetWMIData("ROOT\\CIMV2",
					"SELECT * FROM Win32_BIOS",
					0,
					2,
					"Manufacturer",
					"Version");
	ret->BIOSinfo->Manufacturer = *(tmp2+0);
	ret->BIOSinfo->Version = *(tmp2+1);
	free(tmp2);
#else
	char* MB_info = GetMB_Info();
	ret->Manufacturer = GetMBManufacturer();
	ret->SerialNumber = GetMBSerialNumber(MB_info);
	ret->ProductName = GetMBProductNameCmd();
	ret->BIOSinfo->Manufacturer = GetBIOSManufacturer();
	ret->BIOSinfo->Version = GetBIOSVersion();
#endif
	return ret;
}

MB_DYNAMIC_INFO* lib_SysInfo_GetMBDyInfo()
{
	return NULL;
}

