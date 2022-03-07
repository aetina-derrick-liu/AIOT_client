#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CMDParser.h"
#include "OSInfo.h"
#include <ctype.h>
#ifndef WIN32
char* GetOSName()
{
	char* ret;

	ret = MEMAllocting(__func__);	
	
	GetCommandResult(ret, "cat /etc/*-release | grep PRETTY_NAME", "\"", 1);

	return MEMReallocting(ret);
}

char* GetOSVersion()
{
	char* ret;

	ret = MEMAllocting(__func__);	
	
	GetCommandResult(ret, "cat /etc/*-release | grep VERSION", "\"", 1);

	return MEMReallocting(ret);
}

char* GetOSArchitecture()
{
	char* ret;
	
	ret = MEMAllocting(__func__);
	
	GetCommandResult(ret, "uname -mrs", NULL, 0);

	return MEMReallocting(ret);
}

char* GetHostName()
{
	char* ret;

	ret = MEMAllocting(__func__);	
	
	GetCommandResult(ret, "hostname", NULL, 0);

	return MEMReallocting(ret);
}
#endif

OS_INFO* lib_SysInfo_GetOSInfo()
{
	OS_INFO* ret = (OS_INFO*)malloc(sizeof(OS_INFO));
#ifdef WIN32
	char** tmp = GetWMIData("ROOT\\CIMV2",
							"SELECT * FROM Win32_OperatingSystem",
							0,
							4,
							"Caption",
							"CSDVersion",
							"OSArchitecture",
							"CSName");
/*
	if(isascii(*(tmp+0)))
	{
		ret->Name = *(tmp+0);
	}
	else
	{
		ret->Name ="";
	}
*/
	ret->Name = *(tmp+0);
	ret->Version = *(tmp+1);
	ret->Arch = *(tmp+2);
	ret->HostName = *(tmp+3);
	free(tmp);
#else
	ret->Name = GetOSName();
	ret->Version = GetOSVersion();
	ret->Arch = GetOSArchitecture();
	ret->HostName = GetHostName();
#endif
	return ret;
}
