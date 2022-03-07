#ifndef __LIB_SYSINFO_H__
#define __LIB_SYSINFO_H__

#include <stdbool.h>
#ifdef WIN32
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long long ULLONG;
#else
#define USHORT unsigned short
#define UINT unsigned int
#endif

typedef struct
{
	char* Name;
	char* Version;
	char* Arch;
	char* HostName;
}OS_INFO;

typedef struct
{
	char* Manufacturer;
	char* Name;
	USHORT NumberOfCore;
	USHORT L2Size;
	USHORT L3Size;
}CPU_INFO;

typedef struct
{
	char* Manufacturer;
	char* Version;
}BIOS_INFO;

typedef struct
{
	char* Manufacturer;
	char* ProductName;
	char* SerialNumber;
	BIOS_INFO* BIOSinfo;
}MB_INFO;

typedef struct
{
	char *sICGrade;
    int  AntiSul;          /* 0:No 1:Yes 2:RDIMM/LRDIMM */ 
    bool b30GoldenFing; 
    bool b45GoldenFing; 
    bool bWP;   

}MEMSLOT_FEATURE;
typedef struct
{
	void* prev_node;
	USHORT Index;
	char* PN;
	char* Type;
	char* DIMMType;
	char* Manufacturer;
	char* Location;
	char* OPTemp;
	char* IC_Cfg;
	char* IC_Brand;
	char* Thermal;
	float CAS_Ltc;
	UINT Capacity;
	char* SN;
	char* Date;
	int Rate;
	MEMSLOT_FEATURE* feature;
	void* next_node;
}MEMSLOT_INFO;

typedef struct
{
	UINT Capacity;
	USHORT NumberOfMEM;
	MEMSLOT_INFO* MEMSlot;
}MEM_INFO;

typedef struct
{
	void* prev_node;
	USHORT Index;
	char* MountPoint;
#ifdef WIN32
	ULLONG Capacity;
#else
	UINT Capacity;
#endif
	void* next_node;
}STORAGE_PAR_INFO;

typedef struct
{
	void* prev_node;
	USHORT Index;
	char* Location;
#ifdef WIN32
	ULLONG Capacity;
#else
	UINT Capacity;
#endif
	USHORT NumberOfPartition;
	STORAGE_PAR_INFO* PARinfo;
	void* next_node;
}STORAGE_INFO;

typedef struct
{
	void* prev_node;
	USHORT Index;
	char* Name;
	char* Type;
	char* MACAddress;
	char* IPv4Address;
	char* Netmask;
	char* IPv6Address;
	void* next_node;
}NET_INFO;

/* Allen 2017.11.24 +++ */
typedef struct
{
    char* DeviceName;
}DISPLAY_INFO;

typedef struct
{
	void* prev_node;
	USHORT Index;
	char* Description;
	unsigned char* MACAddress;
    char* IPAddress;
    char* IPMask;
    char* Gateway;
	void* next_node;
}NETCARD_INFO;

/* mental 2020/12/17*/
#ifdef __aarch64__

typedef struct
{
	char* Name;
	char* Arch;
	char* DriverVer;
	char* ComputeCap;
	char* CoreNum;
	char* MemType;
	char* MemBusWidth;
	char* MemSize;
	char* MemBandWidth;
	char* Clock;
	char* MemClock;


}GPU_INFO;

typedef struct
{
	int CoreClock;
	int Temp;
	int MemUsed;
	int Load;
	int FanTemp;
}GPU_DYNAMIC_INFO;
#endif

/* Allen 2017.11.24 --- */

typedef struct
{
	OS_INFO* osinfo;
	CPU_INFO* cpuinfo;
	MB_INFO* mbinfo;
	MEM_INFO* meminfo;
	STORAGE_INFO* storinfo;
	NET_INFO* netinfo;
	GPU_INFO* gpuinfo; //mental 2020.12.17

    /* Allen 2017.11.24 +++ */
    // DISPLAY_INFO* displayinfo;
    // NETCARD_INFO* netcardinfo;
    /* Allen 2017.11.24 --- */
}SYS_INFO;

typedef struct
{
	void* prev_node;
	USHORT Frequency;
	float Usage;
	float Temperature;
	float Voltage;
	void* next_node;
}CPU_CORE_INFO;

typedef struct
{
	USHORT Frequency;
	float Usage;
	USHORT FanRPM;
	CPU_CORE_INFO* core;
}CPU_DYNAMIC_INFO;

typedef struct
{
	void* prev_node;
	USHORT Index;
	float Value;
	void* next_node;
}MB_DYNAMIC_INFO;

typedef struct
{
	
	CPU_DYNAMIC_INFO* cpuinfo;
	MB_DYNAMIC_INFO* mbinfo;
	STORAGE_INFO* storinfo;
	GPU_DYNAMIC_INFO* gpuinfo;
	UINT memless;
	float memtemp;
#ifdef WIN32
	char* eapiinfo;
#endif
}SYS_DYNAMIC_INFO;



char* lib_SysInfo_GetVersion(void);
SYS_INFO* lib_SysInfo_GetSysInfo(void);
SYS_DYNAMIC_INFO* lib_SysInfo_GetSysDyInfo(void);
void lib_SysInfo_free(SYS_INFO* sysinfo);
void lib_SysInfo_free_DyInfo(SYS_DYNAMIC_INFO* sysdyinfo);

char* lib_SysInfo_TrimWhiteSpace(char* str);

#endif
