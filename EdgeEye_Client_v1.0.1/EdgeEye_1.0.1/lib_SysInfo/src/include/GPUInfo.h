#ifndef __GPUINFO_H__
#define __GPUINFO_H__

#include "lib_SysInfo.h"

#ifdef __aarch64__
char* GetGPU_Info();


char* GetGPU_Name(char* GPU_Info);
char* GetGPU_Arch(char* GPU_Info);
char* GetGPU_DriverVer(char* GPU_Info);
char* GetGPU_ComputeCap(char* GPU_Info);
char* GetGPU_CoreNum(char* GPU_Info);
char* GetGPU_MemType(char* GPU_Info);
char* GetGPU_MemBusWidth(char* GPU_Info);
char* GetGPU_MemSize(char* GPU_Info);
char* GetGPU_MemBandWidth(char* GPU_Info);
char* GetGPU_Clock(char* GPU_Info);
char* GetGPU_MemClock(char* GPU_Info);

char* GetGPU_DyInfo();
int GetGPUDy_CoreClock(char* GPUDy_Info);
int GetGPUDy_Temp(char* GPUDy_Info);
int GetGPUDy_MemUsed(char* GPUDy_Info);
int GetGPUDy_Load(char* GPUDy_Info);
int GetGPUDy_FanTemp(char* GPUDy_Info);
#endif

GPU_INFO* lib_SysInfo_GetGPUInfo();
GPU_DYNAMIC_INFO* lib_SysInfo_GetGPUDyInfo();

#endif