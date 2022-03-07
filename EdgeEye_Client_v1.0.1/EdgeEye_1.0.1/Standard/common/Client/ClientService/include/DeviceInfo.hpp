#ifndef __DEVICEINFO_HPP__
#define __DEVICEINFO_HPP__

extern "C"
{
    #include "json-c/json.h"
    #include "lib_SysInfo.h"
}

struct json_object* GetOSInfo(OS_INFO* ptr);
struct json_object* GetCPUInfo(CPU_INFO* ptr);
struct json_object* GetMBInfo(MB_INFO* ptr);
struct json_object* GetMEMInfo(MEM_INFO* ptr);
struct json_object* GetStorageInfo(STORAGE_INFO* ptr);
struct json_object* GetNETInfo(NET_INFO* ptr);
struct json_object* GetCPUDynamicInfo(CPU_DYNAMIC_INFO* ptr);
struct json_object* GetMBDynamicInfo(MB_DYNAMIC_INFO* ptr);
struct json_object* GetStorageDynamicInfo(STORAGE_INFO* ptr);
struct json_object* GetEApiDynamicInfo(char* json_str);
struct json_object* GetGPUInfo(GPU_INFO* ptr);
struct json_object* GetGPUDynamicInfo(GPU_DYNAMIC_INFO* ptr);
int CheckStorChange();
int CheckInno(STORAGE_INFO* ptr);
void GetLocalStor(STORAGE_INFO* ptr);
#endif
