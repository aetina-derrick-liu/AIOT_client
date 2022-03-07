#ifndef __CPUINFO_H__
#define __CPUINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
char* GetCPUManufacturer();
char* GetCPUName();
USHORT GetNumberOfCore();
USHORT GetL2Size();
USHORT GetL3Size();
USHORT GetCPUFrequency();
float GetCPUUsage();
USHORT GetFanRPM();
USHORT GetCoreFrequency(USHORT index);
float GetCoreUsage(USHORT index);
float GetCoreTemperature(USHORT index);
float GetCoreVoltage(USHORT index);
#endif

CPU_INFO* lib_SysInfo_GetCPUInfo();
CPU_DYNAMIC_INFO* lib_SysInfo_GetCPUDyInfo();

#endif
