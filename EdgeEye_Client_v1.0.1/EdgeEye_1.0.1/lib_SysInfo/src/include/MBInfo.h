#ifndef __MBINFO_H__
#define __MBINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
char* GetMB_Info();

char* GetMBManufacturer();
char* GetMBProductName(char* MB_Info);
char* GetMBSerialNumber(char* MB_Info);
char* GetMBProductNameCmd();
char* GetMBSerialNumberCmd();
char* GetBIOSManufacturer();
char* GetBIOSVersion();
#endif
MB_INFO* lib_SysInfo_GetMBInfo();
MB_DYNAMIC_INFO* lib_SysInfo_GetMBDyInfo();

#endif
