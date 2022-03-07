#ifndef __MEMINFO_H__
#define __MEMINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
UINT GetMEMCapacity();
USHORT GetNumberOfMEM();
char* GetMEMSingleType(USHORT index);
char* GetMEMSingleManufacturer(USHORT index);
char* GetMEMSingleLocation(USHORT index);
UINT GetMEMSingleCapacity(USHORT index);
USHORT GetMEMSingleFrequency(USHORT index);
float GetMEMSingleVoltage(USHORT index);
char* GetMEMSingleSN(USHORT index);
#endif
MEM_INFO* lib_SysInfo_GetMEMInfo();
UINT lib_SysInfo_GetMEMLess();
float lib_SysInfo_GetMEMTemp();
#endif
