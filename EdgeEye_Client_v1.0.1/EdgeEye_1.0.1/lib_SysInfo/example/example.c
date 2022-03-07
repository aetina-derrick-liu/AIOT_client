#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "lib_SysInfo.h"

int main(int argc, char** argv)
{
	char* ver;
	SYS_INFO* ptr;
	MEMSLOT_INFO* memptr;
	STORAGE_INFO* storageptr;
	STORAGE_PAR_INFO* storageparptr;
	NET_INFO* netptr;
    /* Allen 2017.11.24 +++ */
    NETCARD_INFO* netcardptr;
    /* Allen 2017.11.24 --- */
	int i;

#ifndef WIN32
	if(getuid() != 0)
	{
		printf("Please run this program with root premission.\n");
		return 0;
	}
#endif

	ver = lib_SysInfo_GetVersion();
	
	if(ver == NULL)
	{
		fprintf(stderr, "Get libraray version fail.\n");
		return -1;
	}
	printf("Get library version:%s\n", ver);

	ptr = lib_SysInfo_GetSysInfo();

	if(ptr == NULL)
	{
		fprintf(stderr, "Get system information fail.\n");
		return -1;
	}
	printf("Get system information success.\n");

	printf("=====Operating System information=====\n");
	printf("OS name:%s\n", ptr->osinfo->Name);
	printf("OS version:%s\n", ptr->osinfo->Version);
	printf("OS architecture:%s\n", ptr->osinfo->Arch);
	printf("Host name:%s\n", ptr->osinfo->HostName);

	printf("=====CPU information=====\n");
	printf("CPU manufacturer:%s\n", ptr->cpuinfo->Manufacturer);
	printf("CPU name:%s\n", ptr->cpuinfo->Name);
	printf("Number of CPU cores:%u\n", ptr->cpuinfo->NumberOfCore);
	printf("CPU L2 cache size:%u KB\n", ptr->cpuinfo->L2Size);
	printf("CPU L3 cache size:%u KB\n", ptr->cpuinfo->L3Size);

	printf("=====Motherboard information=====\n");
	printf("Motherboard manufacturer:%s\n", ptr->mbinfo->Manufacturer);
	printf("Motherboard product name:%s\n", ptr->mbinfo->ProductName);
	printf("Motherboard serial number:%s\n", ptr->mbinfo->SerialNumber);
	printf("BIOS manufacturer:%s\n", ptr->mbinfo->BIOSinfo->Manufacturer);
	printf("BIOS version:%s\n", ptr->mbinfo->BIOSinfo->Version);

	printf("=====Memory information=====\n");
	printf("Memory total capacity:%u KB\n", ptr->meminfo->Capacity);
	printf("Number of memory slot:%u\n", ptr->meminfo->NumberOfMEM);
	memptr = ptr->meminfo->MEMSlot;
	while(memptr != NULL)
	{
		printf("Memory slot %u manufacturer:%s\n", memptr->Index, memptr->Manufacturer);
		printf("Memory slot %u location:%s\n", memptr->Index, memptr->Location);
		printf("Memory slot %u capacity:%u KB\n", memptr->Index, memptr->Capacity);
		memptr = (MEMSLOT_INFO*)memptr->next_node;
	}
	
	printf("=====Storage information=====\n");
	storageptr = ptr->storinfo;
	while(storageptr != NULL)
	{
		printf("Storage %u location:%s\n", storageptr->Index, storageptr->Location);
		printf("Storage %u capacity:%u KB\n", storageptr->Index, storageptr->Capacity);
		printf("Storage %u number of partition:%d\n", storageptr->Index, storageptr->NumberOfPartition);
		if(storageptr->NumberOfPartition > 0)
		{
			storageparptr = storageptr->PARinfo;
			while(storageparptr != NULL)
			{
				printf("Storage %u partition %u mount point:%s\n", storageptr->Index, storageparptr->Index, storageparptr->MountPoint);
				printf("Storage %u partition %u capacity: %u KB\n", storageptr->Index, storageparptr->Index, storageparptr->Capacity);
				storageparptr = (STORAGE_PAR_INFO*)storageparptr->next_node;
			}
		}
		storageptr = (STORAGE_INFO*)storageptr->next_node;
	}	

	printf("=====Network card information=====\n");
	netptr = ptr->netinfo;
	while(netptr != NULL)
	{
		printf("Network card %u name:%s\n", netptr->Index, netptr->Name);
		printf("Network card %u type:%s\n", netptr->Index, netptr->Type);
		printf("Network card %u MAC address:%s\n", netptr->Index, netptr->MACAddress);
		printf("Network card %u IPv4 address:%s\n", netptr->Index, netptr->IPv4Address);
		printf("Network card %u IPv4 netmask:%s\n", netptr->Index, netptr->Netmask);
		printf("Network card %u IPv6 address:%s\n", netptr->Index, netptr->IPv6Address);
		netptr = (NET_INFO*)netptr->next_node;
	}

#ifdef WIN32
    /* Allen 2017.11.24 +++ */
	printf("=====Display information=====\n");
    printf("Display :%s\n", ptr->displayinfo->DeviceName);

    netcardptr = ptr->netcardinfo;
	printf("=====Netcard information=====\n");
    while(netcardptr != NULL)
    {
        printf("Description :%s\n", netcardptr->Description);

        printf("MACAddress  :%02X-%02X-%02X-%02X-%02X-%02X\n", 
                             netcardptr->MACAddress[0], netcardptr->MACAddress[1],
                             netcardptr->MACAddress[2], netcardptr->MACAddress[3],
                             netcardptr->MACAddress[4], netcardptr->MACAddress[5]);
        
        printf("IPAddress   :%s\n", netcardptr->IPAddress);
        printf("IPMask      :%s\n", netcardptr->IPMask);
        printf("Gateway     :%s\n", netcardptr->Gateway);

        netcardptr = (NETCARD_INFO*)netcardptr->next_node;
    }
    /* Allen 2017.11.24 --- */
#endif

	printf("=====End of program, free all allocated memory.=====\n");
	lib_SysInfo_free(ptr);
	printf("=====Get dynamic information.=====\n");
	SYS_DYNAMIC_INFO* dyptr	= lib_SysInfo_GetSysDyInfo();
	lib_SysInfo_free_DyInfo(dyptr);
	printf("=====free dynamic information.=====\n");
	return 0;
}
