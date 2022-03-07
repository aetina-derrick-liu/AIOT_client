#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "CMDParser.h"
#include "lib_SysInfo.h"
#include "OSInfo.h"
#include "CPUInfo.h"
#include "MBInfo.h"
#include "MEMInfo.h"
#include "StorageInfo.h"
#include "NETInfo.h"
#include "GPUInfo.h"
#ifdef WIN32
#define PASS "  [PASS]"
#else
#define PASS "  [\x1b[32mPASS\x1b[0m]"
#endif

int main(int argc, char** argv)
{
	char* ver, *ptr;
	unsigned short temp;
	unsigned int tempi;
	float tempf;
	int i, memSlotSize, storageSize, netSize;
	clock_t begin_time, end_time;
	OS_INFO* osinfoptr;
	CPU_INFO* cpuinfoptr;
	CPU_DYNAMIC_INFO* cpudyinfoptr;
	CPU_CORE_INFO* current_core;
	MB_INFO* mbinfoptr;
	MEM_INFO* meminfoptr;
	MEMSLOT_INFO* current_slot;
	STORAGE_INFO* storinfoptr;
	STORAGE_PAR_INFO* current_stor;
	NET_INFO* netinfoptr;
	SYS_INFO* sysinfo;
	SYS_DYNAMIC_INFO* dyinfo;

	char* ret = (char*)malloc(sizeof(char) * 500);
	/** Get library version test **/
	begin_time = clock();
	assert((ver = lib_SysInfo_GetVersion()) != NULL);
	end_time = clock();
	printf(PASS"lib_SysInfo_GetVersion(), version:%s, spend time:%.4f seconds.\n", ver, (double)(end_time-begin_time)/ CLOCKS_PER_SEC);

#ifndef WIN32
	/** Get system call command result function tests **/
	assert((GetCommandResult(ret, "uname -a", NULL, 0)) != NULL);
	printf(PASS"GetCommandResult(ret, \"uname -a\", NULL, 0)\n");
	assert((GetCommandResult(ret, "uname -a", " ", 2)) != NULL);
	printf(PASS"GetCommandResult(ret, \"uname -a\", \" \", 2)\n");
	assert((GetCommandResult(ret, "uname -a", " ", 100)) != NULL);
	printf(PASS"GetCommandResult(ret, \"uname -a\", \" \", 100)\n");
	free(ret);
#endif

	/** Get OS information functions tests **/
#ifndef WIN32
	assert((ptr = GetOSName()) != NULL);
	printf(PASS"GetOSName(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetOSVersion()) != NULL);
	printf(PASS"GetOSVersion(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetOSArchitecture()) != NULL);
	printf(PASS"GetOSArchitecture(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetHostName()) != NULL);
	printf(PASS"GetHostName(), result:%s\n", ptr);
	free(ptr);
#endif
	begin_time = clock();
	assert((osinfoptr = lib_SysInfo_GetOSInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetOSInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	free(osinfoptr->Name);
	free(osinfoptr->Version);
	free(osinfoptr->Arch);
	free(osinfoptr->HostName);
	free(osinfoptr);

	/** Get CPU information functions tests **/
#ifndef WIN32
	assert((ptr = GetCPUManufacturer()) != NULL);
	printf(PASS"GetCPUManufacturer(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetCPUName()) != NULL);
	printf(PASS"GetCPUName(), result:%s\n", ptr);
	free(ptr);
	assert((temp = GetNumberOfCore()) > 0);
	printf(PASS"GetNumberOfCore(), result:%u\n", temp);
	assert((temp = GetL2Size()) >= 0);
	printf(PASS"GetL2Size(), result:%u\n", temp);
	assert((temp = GetL3Size()) >= 0);
	printf(PASS"GetL3Size(), result:%u\n", temp);
	assert((temp = GetCPUFrequency()) >= 0);
	printf(PASS"GetCPUFrequency(), result:%u\n", temp);
	assert((tempf = GetCPUUsage()) >= 0);
	printf(PASS"GetCPUUsage(), result:%f\n", tempf);
	assert((temp = GetFanRPM()) >= 0);
	printf(PASS"GetFanRPM(), result:%u\n", temp);
	assert((temp = GetCoreFrequency(0)) >= 0);
	printf(PASS"GetCoreFrequency(0), result:%u\n", temp);
	assert((tempf = GetCoreUsage(0)) >= 0);
	printf(PASS"GetCoreUsage(0), result:%f\n", tempf);
	assert((tempf = GetCoreTemperature(0)) >= 0);
	printf(PASS"GetCoreTemperature(0), result:%f\n", tempf);
	assert((tempf = GetCoreVoltage(0)) >= 0);
	printf(PASS"GetCoreVoltage(0), result:%f\n", tempf);
#endif
	begin_time = clock();
	assert((cpuinfoptr = lib_SysInfo_GetCPUInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetCPUInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	begin_time = clock();
	assert((cpudyinfoptr = lib_SysInfo_GetCPUDyInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetCPUDyInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	free(cpuinfoptr->Manufacturer);
	free(cpuinfoptr->Name);
	free(cpuinfoptr);
	current_core = cpudyinfoptr->core;
	while(current_core != NULL)
	{
		CPU_CORE_INFO* tmp = current_core;
		current_core = tmp->next_node;
		free(tmp);
	}
	free(cpudyinfoptr);

	//Get MB information functions tests 

#ifndef WIN32
	//#ifdef __x86_64__
	assert((ptr = GetMBManufacturer()) != NULL);
	printf(PASS"GetMBManufacturer(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetMBProductNameCmd()) != NULL);
	printf(PASS"GetMBProductName(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetMBSerialNumberCmd()) != NULL);
	printf(PASS"GetMBSerialNumber(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetBIOSManufacturer()) != NULL);
	printf(PASS"GetBIOSManufacturer(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetBIOSVersion()) != NULL);
	printf(PASS"GetBIOSVersion(), result:%s\n", ptr);
	free(ptr);
	//#endif
#endif
#ifndef __aarch64__
	begin_time = clock();
	assert((mbinfoptr = lib_SysInfo_GetMBInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetMBInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	free(mbinfoptr->Manufacturer);
	free(mbinfoptr->ProductName);
	free(mbinfoptr->SerialNumber);
	free(mbinfoptr->BIOSinfo->Manufacturer);
	free(mbinfoptr->BIOSinfo->Version);
	free(mbinfoptr->BIOSinfo);
	free(mbinfoptr);
#endif
	//Get MEM information functions tests

#ifndef WIN32
	assert((tempi = GetMEMCapacity()) > 0);
	printf(PASS"GetMEMCapacity(), result=%u\n", tempi);
	assert((temp = GetNumberOfMEM()) >= 0 );
	printf(PASS"GetNumberOfMEM(), result=%u\n", temp);
	memSlotSize = temp;
	if(memSlotSize > 0)
	{
		for(i = 0; i < memSlotSize; i++)
		{
			assert((ptr = GetMEMSingleManufacturer(i)) != NULL);
			printf(PASS"GetMEMSingleManufacturer(%d), result:%s\n", i, ptr);
			free(ptr);
			assert((ptr = GetMEMSingleLocation(i)) != NULL);
			printf(PASS"GetMEMSingleLocation(%d), result:%s\n", i, ptr);
			free(ptr);	
			assert((tempi = GetMEMSingleCapacity(i)) >= 0);
			printf(PASS"GetMEMSingleCapacity(%d), result=%u\n", i, tempi);
			assert((temp = GetMEMSingleFrequency(i)) >= 0);
			printf(PASS"GetMEMSingleFrequency(%d), result=%u\n", i, temp);
			assert((tempf = GetMEMSingleVoltage(i)) >= 0);
			printf(PASS"GetMEMSingleVoltage(%d), result=%f\n", i, tempf);
		}
	}
#endif

	begin_time = clock();
	assert((meminfoptr = lib_SysInfo_GetMEMInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetMEMInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	current_slot = meminfoptr->MEMSlot;
	while(current_slot != NULL)
	{
		MEMSLOT_INFO* del_slot = current_slot;	
		free(current_slot->Type);
		free(current_slot->Manufacturer);
		free(current_slot->Location);
		free(current_slot->SN);
//		free(current_slot->Date);
		current_slot = (MEMSLOT_INFO*)current_slot->next_node;
		free(del_slot);
	}
	free(meminfoptr);
	begin_time = clock();
	assert((tempi = lib_SysInfo_GetMEMLess()) >= 0);
	end_time = clock();
	printf(PASS"GetMEMLess(), result=%u, spend time:%.4f seconds.\n", tempi, (double)(end_time-begin_time)/ CLOCKS_PER_SEC);

	/** Get Storage information functions tests **/
#ifndef WIN32
#elif __aarch64__
#else
	// assert((temp = GetNumberOfStorage()) > 0);
	// printf(PASS"GetNumberOfStorage(), result=%u\n", temp);
	assert((ptr = GetStorageLocation(0)) != NULL);
	printf(PASS"GetStorageLocation(0), result:%s\n", ptr);
	free(ptr);
	assert(tempi = (GetStorageCapacity("sda") > 0 || GetStorageCapacity("nvme0n1") > 0));
	printf(PASS"GetStorageCapacity(), result=%u\n", tempi);
	assert(temp = (GetNumberOfPartition("sda") >= 0 || GetNumberOfPartition("nvme0n1") >= 0));
	printf(PASS"GetNumberOfPartition(\"sda\") or GetNumberOfPartition(\"nvme0n1\"), result=%u\n", temp);
	assert((ptr = GetPartitionMountPoint("sda",0)) != NULL || (ptr = GetPartitionMountPoint("sda",0)) != NULL);
	printf(PASS"GetPartitionMountPoint(\"sda\", 0) or GetPartitionMountPoint(\"nvme0n1\",0), result:%s\n", ptr);
	free(ptr);
	assert(tempi = (GetPartitionCapacity("sda", 0) >= 0 || GetPartitionCapacity("nvme0n1", 0) >= 0));
	printf(PASS"GetPartitionCapacity(\"sda\", 0) or GetPartitionCapacity(\"nvme0n1\", 0), result=%u\n", tempi);
#endif	
#ifndef __aarch64__
	begin_time = clock();
	// assert((storinfoptr = lib_SysInfo_GetStorageInfo()) != NULL);
	// end_time = clock();
	// printf(PASS"GetStorageInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	// while(storinfoptr != NULL)
	// {
	// 	STORAGE_INFO* del_stor = storinfoptr;
	// 	if(storinfoptr->NumberOfPartition > 0)
	// 	{
	// 		current_stor = storinfoptr->PARinfo;
	// 		while(current_stor != NULL)
	// 		{	
	// 			STORAGE_PAR_INFO* del_stor_par = current_stor;
	// 			free(current_stor->MountPoint);
	// 			current_stor = current_stor->next_node;
	// 			free(del_stor_par);
	// 		}
	// 	}
	// 	free(storinfoptr->Location);
	// 	storinfoptr = storinfoptr->next_node;
	// 	free(del_stor);
	// }
#endif
	/** Get NET information functions tests **/
#ifndef WIN32
	assert((temp = GetNumberOfNETCard()) > 0);
	printf(PASS"GetNumberOfNETCard(), result=%u\n", temp);
	netSize = temp;
	for(i = 0; i < netSize; i++)
	{
		char* nowname = (char*)malloc(sizeof(char) * 200);
		assert((ptr = GetNETName(i)) != NULL);
		printf(PASS"GetNETName(%d), result:%s\n", i, ptr);
		strcpy(nowname, ptr);
		free(ptr);
		assert((ptr = GetNETType(nowname)) != NULL);
		printf(PASS"GetNETType(%d), result:%s\n", i, ptr);
		free(ptr);
		assert((ptr = GetNETMACAddress(nowname)) != NULL);
		printf(PASS"GetNETMACAddress(%d), result:%s\n", i, ptr);
		free(ptr);
		assert((ptr = GetNETIPv4Address(nowname)) != NULL);
		printf(PASS"GetNETIPv4Address(%d), result:%s\n", i, ptr);
		free(ptr);
		assert((ptr = GetNETNetmask(nowname)) != NULL);
		printf(PASS"GetNETNetmask(%d), result:%s\n", i, ptr);
		free(ptr);
		assert((ptr = GetNETIPv6Address(nowname)) != NULL);
		printf(PASS"GetNETIPv6Address(%d), result:%s\n", i, ptr);
		free(ptr);
		free(nowname);
	}
#endif
 /** Get GPU information functions tests **/
 #ifdef __aarch64__
	assert((ptr = GetGPU_Info()) != NULL);
	printf(PASS"GetGPU_Info(), result:%s\n", ptr);
	free(ptr);
	assert((ptr = GetGPU_DyInfo()) != NULL);
	printf(PASS"GetGPU_DyInfo(), result:%s\n", ptr);
	free(ptr);



 #endif
 #if 1
	begin_time = clock();
	assert((netinfoptr = lib_SysInfo_GetNETInfo()) != NULL);
	end_time = clock();
	printf(PASS"GetNETInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	while(netinfoptr != NULL)
	{
		NET_INFO* del_net = netinfoptr;
		free(netinfoptr->Name);
		free(netinfoptr->Type);
		free(netinfoptr->MACAddress);
		free(netinfoptr->IPv4Address);
		free(netinfoptr->Netmask);
		free(netinfoptr->IPv6Address);
		netinfoptr = (NET_INFO*)netinfoptr->next_node;
		free(del_net);
	}	
	/** Compresion test **/
	begin_time = clock();
	assert((sysinfo = lib_SysInfo_GetSysInfo()) != NULL);
	end_time = clock();
	printf(PASS"lib_SysInfo_GetSysInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	begin_time = clock();
	lib_SysInfo_free(sysinfo);
	end_time = clock();
	printf(PASS"lib_SysInfo_free(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	begin_time = clock();
	assert((dyinfo = lib_SysInfo_GetSysDyInfo()) != NULL);
	end_time = clock();
	printf(PASS"lib_SysInfo_GetSysDyInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	begin_time = clock();
	lib_SysInfo_free_DyInfo(dyinfo);
	end_time = clock();
	printf(PASS"lib_SysInfo_free_DyInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
#endif
/*#ifdef __aarch64__
	begin_time = clock();
	lib_SysInfo_GetGPUInfo();
	end_time = clock();
	printf(PASS"lib_SysInfo_GetGPUInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
	begin_time = clock();
	lib_SysInfo_GetGPUDyInfo();
	end_time = clock();
	printf(PASS"lib_SysInfo_GetGPUDyInfo(), spend time:%.4f seconds.\n", (double)(end_time-begin_time)/ CLOCKS_PER_SEC);
#endif*/

	return 0;
}
