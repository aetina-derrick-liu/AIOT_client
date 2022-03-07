#include <stdio.h>
#include <stdlib.h>
#include "lib_SysInfo.h"
#include "OSInfo.h"
#include "MBInfo.h"
#include "MEMInfo.h"
#include "CPUInfo.h"
#include "StorageInfo.h"
#include "NETInfo.h"
#ifdef WIN32
#include "EApiInfo.h"
#endif
#ifdef __aarch64__
#include "GPUInfo.h"
#endif
/* Allen 2017.11.24 +++ */
#include "DisplayInfo.h"
#include "NetcardInfo.h"
/* Allen 2017.11.24 --- */

#ifdef WIN32
	#include "libcpu.h"
	SPD_INFO spd;
#endif

//SYS_INFO* sysinfo = NULL;
//SYS_DYNAMIC_INFO* sysdyinfo = NULL;

char version[] = "1.1.1.0";

char* lib_SysInfo_GetVersion(void)
{
	return &version[0];
}

SYS_INFO* lib_SysInfo_GetSysInfo(void)
{
	SYS_INFO* sysinfo=(SYS_INFO*)malloc(sizeof(SYS_INFO));
	//printf("Start to get system information\n");
	sysinfo->osinfo = lib_SysInfo_GetOSInfo();
	sysinfo->cpuinfo = lib_SysInfo_GetCPUInfo();
	sysinfo->mbinfo = lib_SysInfo_GetMBInfo();
	sysinfo->meminfo = lib_SysInfo_GetMEMInfo();
	sysinfo->storinfo = lib_SysInfo_GetStorageInfo();
	sysinfo->netinfo = lib_SysInfo_GetNETInfo();
	sysinfo->gpuinfo = lib_SysInfo_GetGPUInfo();
    /* Allen 2017.11.24 +++ */
    // sysinfo->displayinfo = lib_SysInfo_GetDisplayInfo();
    // sysinfo->netcardinfo = lib_SysInfo_GetNetcardInfo();
    /* Allen 2017.11.24 --- */

	return sysinfo;
}

SYS_DYNAMIC_INFO* lib_SysInfo_GetSysDyInfo(void)
{
	SYS_DYNAMIC_INFO* sysdyinfo;
	//if(sysdyinfo == NULL)
	{
		sysdyinfo = (SYS_DYNAMIC_INFO*)malloc(sizeof(SYS_DYNAMIC_INFO));
	}
	sysdyinfo->cpuinfo = lib_SysInfo_GetCPUDyInfo();
	sysdyinfo->mbinfo = lib_SysInfo_GetMBDyInfo();
	sysdyinfo->storinfo = lib_SysInfo_GetStorageInfo();
	sysdyinfo->memless = lib_SysInfo_GetMEMLess();
	sysdyinfo->memtemp = lib_SysInfo_GetMEMTemp();
	sysdyinfo->gpuinfo = lib_SysInfo_GetGPUDyInfo();
#ifdef WIN32
	sysdyinfo->eapiinfo = lib_SysInfo_GetEApiInfo();
#endif
	return sysdyinfo;
}

void lib_SysInfo_free(SYS_INFO* sysinfo)
{
	MEMSLOT_INFO* current_slot;
	STORAGE_INFO* current_stor;
	STORAGE_PAR_INFO* current_stor_par;
	NET_INFO* current_net;

    /* Allen 2017.11.24 +++ */
	NETCARD_INFO* current_netcard;
    /* Allen 2017.11.24 --- */

	if(sysinfo != NULL)
	{
		free(sysinfo->osinfo->Name);
		free(sysinfo->osinfo->Version);
		free(sysinfo->osinfo->Arch);
		free(sysinfo->osinfo->HostName);
		free(sysinfo->osinfo);
		free(sysinfo->cpuinfo->Manufacturer);
		free(sysinfo->cpuinfo->Name);
		free(sysinfo->cpuinfo);
		free(sysinfo->mbinfo->Manufacturer);
		free(sysinfo->mbinfo->ProductName);
		free(sysinfo->mbinfo->SerialNumber);
		free(sysinfo->mbinfo->BIOSinfo->Manufacturer);
		free(sysinfo->mbinfo->BIOSinfo->Version);
		free(sysinfo->mbinfo->BIOSinfo);
		free(sysinfo->mbinfo);
		current_slot = sysinfo->meminfo->MEMSlot;
		while(current_slot != NULL)
		{
			MEMSLOT_INFO* del_slot = current_slot;
#ifdef WIN32
			free(current_slot->Type);
			free(current_slot->DIMMType);
			free(current_slot->Manufacturer);
			free(current_slot->Location);
			free(current_slot->OPTemp);
			free(current_slot->IC_Cfg);
			free(current_slot->IC_Brand);
			free(current_slot->Thermal);
			free(current_slot->SN);
			free(current_slot->Date);
			free(current_slot->feature->sICGrade);
#endif
			current_slot = (MEMSLOT_INFO*)current_slot->next_node;
			free(del_slot);
		}
		free(sysinfo->meminfo);

		current_stor = sysinfo->storinfo;
		while(current_stor != NULL)
		{
			STORAGE_INFO* del_stor = current_stor;
			if(current_stor->NumberOfPartition > 0)
			{
				current_stor_par = current_stor->PARinfo;
				while(current_stor_par != NULL)
				{
					STORAGE_PAR_INFO* del_stor_par = current_stor_par;
					free(current_stor_par->MountPoint);
					current_stor_par = current_stor_par->next_node;
					free(del_stor_par);
				}
			}
			free(current_stor->Location);
			current_stor = (STORAGE_INFO*)current_stor->next_node;
			free(del_stor);
		}

		current_net = sysinfo->netinfo;
		while(current_net != NULL)
		{
			NET_INFO* del_net = current_net;
			free(current_net->Name);
			free(current_net->Type);
			free(current_net->MACAddress);
			free(current_net->IPv4Address);
			free(current_net->Netmask);
			free(current_net->IPv6Address);
			current_net = (NET_INFO*)current_net->next_node;
			free(del_net);
		}

#if 0
#ifdef WIN32
        /* Allen 2017.11.24 +++ */
        free(sysinfo->displayinfo->DeviceName);
        free(sysinfo->displayinfo);

		current_netcard = sysinfo->netcardinfo;
		while(current_netcard != NULL)
		{
			NETCARD_INFO* del_node = current_netcard;
			free(current_netcard->Description);
			free(current_netcard->MACAddress);
			free(current_netcard->IPAddress);
			free(current_netcard->IPMask);
			free(current_netcard->Gateway);
			current_netcard = (NETCARD_INFO*)current_netcard->next_node;
			free(del_node);
		}
		free(sysinfo->netcardinfo);
        /* Allen 2017.11.24 --- */
#else
        free(sysinfo->displayinfo);
        free(sysinfo->netcardinfo);       
#endif
#endif

#ifdef WIN32
		if(spd.module)	free_spd_info(&spd);
    	free_lib();
#endif
		free(sysinfo);
	}
}

void lib_SysInfo_free_DyInfo(SYS_DYNAMIC_INFO* sysdyinfo)
{
	if (sysdyinfo != NULL) {
		// free cpuinfo
		CPU_CORE_INFO* current_core = sysdyinfo->cpuinfo->core;
		while (current_core != NULL) {
			CPU_CORE_INFO* delete_core = current_core;
			current_core = current_core->next_node;
			free(delete_core);
		}
		if (sysdyinfo->cpuinfo != NULL) {
			free(sysdyinfo->cpuinfo);
			sysdyinfo->cpuinfo = NULL;
		}
		// free mbinfo
		MB_DYNAMIC_INFO* current_mbinfo = sysdyinfo->mbinfo;
		while (current_mbinfo != NULL) {
			MB_DYNAMIC_INFO* delete_mbinfo = current_mbinfo;
			current_mbinfo = current_mbinfo->next_node;
			free(delete_mbinfo);
		}
		
		// free storinfo
		STORAGE_INFO* current_stor = sysdyinfo->storinfo;
		while (current_stor != NULL) {
			STORAGE_PAR_INFO* current_par = current_stor->PARinfo;
			STORAGE_PAR_INFO* tmp_par;
			while (current_par != NULL) {
				tmp_par = current_par;
				current_par = current_par->next_node;
				free(tmp_par->MountPoint);
				free(tmp_par);
			}
			free (current_stor->Location);
			STORAGE_INFO* tmp_stor = current_stor;
			current_stor = tmp_stor->next_node;
			free(tmp_stor);
		}

#ifdef WIN32
		// free eapiinfo
		if (sysdyinfo->eapiinfo != NULL) {
			free(sysdyinfo->eapiinfo);
			sysdyinfo->eapiinfo = NULL;
		}
#endif
		
		// free sysdyinfo
		free(sysdyinfo);
		sysdyinfo = NULL;
	}
}
