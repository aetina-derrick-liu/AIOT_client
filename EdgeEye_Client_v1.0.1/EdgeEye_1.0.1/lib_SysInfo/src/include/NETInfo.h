#ifndef __NETINFO_H__
#define __NETINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
USHORT GetNumberOfNETCard();
char* GetNETName(USHORT Index);
char* GetNETType(char* Name);
char* GetNETManufacturer(USHORT Index);
char* GetNETProductName(USHORT Index);
char* GetNETMACAddress(char* Name);
char* GetNETIPv4Address(char* Name);
char* GetNETNetmask(char* Name);
char* GetNETIPv6Address(char* Name);
#endif
NET_INFO* lib_SysInfo_GetNETInfo();

#endif
