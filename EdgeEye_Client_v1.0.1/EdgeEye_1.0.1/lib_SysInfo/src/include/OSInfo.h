#ifndef __OSINFO_H__
#define __OSINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
char* GetOSName();
char* GetOSVersion();
char* GetOSArchitecture();
char* GetHostName();
#endif

OS_INFO* lib_SysInfo_GetOSInfo();

#endif
