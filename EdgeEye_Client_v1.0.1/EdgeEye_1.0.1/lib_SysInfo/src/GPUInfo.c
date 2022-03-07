#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CMDParser.h"
#include "GPUInfo.h"
#ifdef WIN32
	#include "libcpu.h"
	extern SPD_INFO spd;
#endif

#ifdef __aarch64__
char* GetGPU_Info()
{
	char* ret;

	ret = MEMAllocting(__func__);
	//ret = strcpy(ret, "GPU_Info");
	GetCommandResult(ret, "/opt/Jetson_status/Jetson_status GPUInfo", NULL, 1);

	return MEMReallocting(ret);
}

char* GetGPU_Name(char* GPU_Info){
	char* ret;
	char* item = "Name";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,8);
	ret = strcpy(ret,result);
	return MEMReallocting(ret);

}
char* GetGPU_Arch(char* GPU_Info){
	char* ret;
	char* item = "Arch";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,8);
	ret = strcpy(ret,result);
	return MEMReallocting(ret);
}
char* GetGPU_DriverVer(char* GPU_Info){
	char* ret;
	char* item = "DriverVer";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,13);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_ComputeCap(char* GPU_Info){
	char* ret;
	char* item = "ComputeCap";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,14);
	ret = strcpy(ret,result);
	printf("ComputeCap is %s\n",ret);
	return MEMReallocting(ret);
}
char* GetGPU_CoreNum(char* GPU_Info){
	char* ret;
	char* item = "CoreNum";
	char* result;
	ret = MEMAllocting(__func__);	
	result = GetJetsonResult(GPU_Info,item,11);
	ret = strcpy(ret,result);
	
	return MEMReallocting(ret);
}
char* GetGPU_MemType(char* GPU_Info){
	char* ret;
	char* item = "MemType";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,11);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_MemBusWidth(char* GPU_Info){
	char* ret;
	char* item = "MemBusWidth";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,15);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_MemSize(char* GPU_Info){
	char* ret;
	char* item = "MemSize";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,11);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_MemBandWidth(char* GPU_Info){
	char* ret;
	char* item = "MemBandWidth";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,16);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_Clock(char* GPU_Info){
	char* ret;
	char* item = "Clock";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,9);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}
char* GetGPU_MemClock(char* GPU_Info){
	char* ret;
	char* item = "MemClock";
	char* result;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult(GPU_Info,item,12);
	ret = strcpy(ret,result);

	return MEMReallocting(ret);
}

char* GetGPU_DyInfo()
{
	char* ret;
	
	ret = MEMAllocting(__func__);
	//ret = strcpy(ret, "GetGPU_DyInfo");
	GetCommandResult(ret, "/opt/Jetson_status/Jetson_status GPUDyInfo",NULL, 1);
	
	return MEMReallocting(ret);

}

int GetGPUDy_CoreClock(char* GPUDy_Info){
	char* ret;
	char* item = "CoreClock";
	char* result;
	int resutlt_value;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult_int(GPUDy_Info,item,12);
	
	ret = strcpy(ret,result);
	resutlt_value = atoi(ret);
	MEMReallocting(ret);
	return resutlt_value;
}
int GetGPUDy_Temp(char* GPUDy_Info){
	char* ret;
	char* item = "Temp";
	char* result;
	int resutlt_value;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult_int(GPUDy_Info,item,7);
	ret = strcpy(ret,result);
	resutlt_value = atoi(ret);
	MEMReallocting(ret);
	return resutlt_value;
}
int GetGPUDy_MemUsed(char* GPUDy_Info){
	char* ret;
	char* item = "MemUsed";
	char* result;
	int resutlt_value;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult_int(GPUDy_Info,item,10);
	ret = strcpy(ret,result);
	resutlt_value = atoi(ret);
	MEMReallocting(ret);
	return resutlt_value;
}
int GetGPUDy_Load(char* GPUDy_Info){
	char* ret;
	char* item = "Load";
	char* result;
	int resutlt_value;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult_int(GPUDy_Info,item,7);
	ret = strcpy(ret,result);
	resutlt_value = atoi(ret);
	MEMReallocting(ret);
	return resutlt_value;
}
int GetGPUDy_FanTemp(char* GPUDy_Info){
	char* ret;
	char* item = "FanTemp";
	char* result;
	int resutlt_value;
	ret = MEMAllocting(__func__);	

	result = GetJetsonResult_int(GPUDy_Info,item,10);
	ret = strcpy(ret,result);
	resutlt_value = atoi(ret);
	MEMReallocting(ret);
	return resutlt_value;
}
#endif

/***************************************************************/
#define BUF_MAX 1024

GPU_INFO* lib_SysInfo_GetGPUInfo()
{
	GPU_INFO* gpuinfoptr = (GPU_INFO*)malloc(sizeof(GPU_INFO));
#ifdef WIN32

#elif __aarch64__
	char* GPU_info = GetGPU_Info();
	//printf("%s\n",GPU_info);
	gpuinfoptr->Name = GetGPU_Name(GPU_info);
	gpuinfoptr->Arch = GetGPU_Arch(GPU_info);
	gpuinfoptr->DriverVer = GetGPU_DriverVer(GPU_info);
	gpuinfoptr->ComputeCap = GetGPU_ComputeCap(GPU_info);
	gpuinfoptr->CoreNum = GetGPU_CoreNum(GPU_info);
	gpuinfoptr->MemType = GetGPU_MemType(GPU_info);
	gpuinfoptr->MemBusWidth = GetGPU_MemBusWidth(GPU_info);
	gpuinfoptr->MemSize = GetGPU_MemSize(GPU_info);
	gpuinfoptr->MemBandWidth = GetGPU_MemBandWidth(GPU_info);
	gpuinfoptr->Clock = GetGPU_Clock(GPU_info);
	gpuinfoptr->MemClock = GetGPU_MemClock(GPU_info);
	/*printf("---GPUInfo---\n");
	printf("Name is %s\n",gpuinfoptr->Name);
	printf("Arch is %s\n",gpuinfoptr->Arch);
	printf("DriverVer is %s\n",gpuinfoptr->DriverVer);
	printf("ComputeCap is %s\n",gpuinfoptr->ComputeCap);
	printf("CoreNum is %s\n",gpuinfoptr->CoreNum);
	printf("MemType is %s\n",gpuinfoptr->MemType);
	printf("MemBusWidth is %s\n",gpuinfoptr->MemBusWidth);
	printf("MemSize is %s\n",gpuinfoptr->MemSize);
	printf("MemBandWidth is %s\n",gpuinfoptr->MemBandWidth);
	printf("Clock is %s\n",gpuinfoptr->Clock);
	printf("MemClock is %s\n",gpuinfoptr->MemClock);*/

#endif
	return gpuinfoptr;
}

GPU_DYNAMIC_INFO* lib_SysInfo_GetGPUDyInfo()
{
	GPU_DYNAMIC_INFO* ptr = (CPU_DYNAMIC_INFO*)malloc(sizeof(CPU_DYNAMIC_INFO));
#ifdef __aarch64__

	char* GPUDy_info = GetGPU_DyInfo();
	ptr->CoreClock = GetGPUDy_CoreClock(GPUDy_info);
	ptr->Temp = GetGPUDy_Temp(GPUDy_info);
	ptr->MemUsed = GetGPUDy_MemUsed(GPUDy_info);
	ptr->Load = GetGPUDy_Load(GPUDy_info);
	ptr->FanTemp = GetGPUDy_FanTemp(GPUDy_info);
	/*printf("---GPUDyInfo---\n");
	printf("CoreClock is %d\n",ptr->CoreClock);
	printf("Temp is %d\n",ptr->Temp);
	printf("MemUsed is %d\n",ptr->MemUsed);
	printf("Load is %d\n",ptr->Load);
	printf("FanTemp is %d\n",ptr->FanTemp);*/

#endif
	return ptr;
}

