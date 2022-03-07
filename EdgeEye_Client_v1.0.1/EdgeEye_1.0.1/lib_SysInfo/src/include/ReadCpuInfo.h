#pragma once
#ifdef WIN32
#include<windows.h>
#pragma comment (lib,"WinRing0.lib")
#define CONSOLE_PRINT 1

static char fileName[30];
static int cpuTemp[8];
static int coreNumber;
static int Tjmax;

void ReadCpuInfoInit();
void ReadCpuInfoExit();

void ReadCpuInfo();
void ReadCpuTemp();
void CpuTempDisplay();
void SysTimeDisplay();
void clearRecordFile();
float GetCoreTemp(int core_index);
#endif
