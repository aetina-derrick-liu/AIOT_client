#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "CMDParser.h"
#include "StorageInfo.h"
#include <math.h>
#ifndef WIN32


char* GetStorageLocation(USHORT index)
{
	char* ret, *cmd, *realloc_ret;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "lsblk -o NAME,TYPE,TRAN | grep -E 'disk.*sata|disk.*nvme' | tail -n +%u", index +1);

	GetCommandResult(ret, cmd, " ", 0);

	memset(cmd, 0, MAX_CMD_STR_LEN);

	realloc_ret = MEMReallocting(ret);

	sprintf(cmd, "/dev/%s", realloc_ret);

	free(realloc_ret);
	
	return MEMReallocting(cmd);
}

UINT GetStorageCapacity(char* loc)
{
	char *ret, *cmd, *unit;
	UINT retValue;	

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
	unit = MEMAllocting(__func__);


	sprintf(cmd, "lsblk | grep %s", loc);

	GetCommandResult(ret, cmd, " GM", 3);
	GetCommandResult(unit, cmd, " ", 3);

	if(strcmp(&(*(unit+(strlen(unit)-1))), "G") == 0)
	{
		retValue = atoi(ret) * 1024 * 1024;
	}
	else if(strcmp(&(*(unit+(strlen(unit)-1))), "M") == 0)
	{
		retValue = atoi(ret) * 1024;
	}
	else
	{
		retValue = atoi(ret);
	}
	
	free(ret);
	free(cmd);
	free(unit);
	
	return retValue;
}

USHORT GetNumberOfPartition(char* loc)
{
	char *ret, *cmd;
	USHORT retValue;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);

	if(ret == NULL || cmd == NULL)
	{
		fprintf(stderr, "[%s]Allocating memory fail.\n", __func__);
	}

	sprintf(cmd, "lsblk | grep part | grep %s -c", loc);

	GetCommandResult(ret, cmd, NULL, 0);

	retValue = atoi(ret);

	free(ret);
	free(cmd);

	return retValue;
}


UINT GetPartitionCapacity(char* loc, int index)
{
	char *ret, *cmd, *unit;
	UINT retValue;	

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
	unit = MEMAllocting(__func__);

	sprintf(cmd, "lsblk | grep part | grep %s | tail -n +%d", loc, index + 1);
	
	GetCommandResult(ret, cmd, " GM", 3);
	GetCommandResult(unit, cmd, " ", 3);

	if(strcmp(&(*(unit+(strlen(unit)-1))), "G") == 0)
	{
		retValue = atoi(ret) * 1024 * 1024;
	}
	else if(strcmp(&(*(unit+(strlen(unit)-1))), "M") == 0)
	{
		retValue = atoi(ret) * 1024;
	}
	else
	{
		retValue = atoi(ret);
	}
	
	free(ret);
	free(cmd);
	free(unit);
	
	return retValue;

}
#else
static int Partition_Label[24];
static double Partition_Size[24];
static double Partition_Free_Size[24];

int Get_Device_Partition_Number(int devID)
{
	HANDLE hDrv = INVALID_HANDLE_VALUE;
	DWORD	dReturn;

	VOLUME_DISK_EXTENTS my_vd_ex;
	DISK_EXTENT	*pDISK_EXTENT;

	BOOL bResult;

	char guidbuf[512], drv_name[64];

	unsigned __int64 i64TotalBytes, i64FreeBytes;

	unsigned char i, j;

	char *str;
	str = (char*)malloc(sizeof(char) * 50);
	int ret = 0;

	memset(drv_name, 0, 64);
	memset(guidbuf, 0, 512);

	for (i = 0, j = 0; i<24; i++)
	{
		sprintf(drv_name, "%c:\\", i + 0x43);

		if (GetVolumeNameForVolumeMountPointA(drv_name, &guidbuf[0], sizeof(guidbuf)))
		{
			guidbuf[strlen(guidbuf) - 1] = 0;

			hDrv = CreateFileA(guidbuf,
							   GENERIC_READ,
							   FILE_READ_DATA | FILE_WRITE_DATA,
							   0,
							   OPEN_EXISTING,
							   0,
							   0);

			if (hDrv != INVALID_HANDLE_VALUE)
			{
				bResult = DeviceIoControl(hDrv,
										  IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
										  NULL,
										  0,
										  &my_vd_ex,
										  sizeof(my_vd_ex),
										  &dReturn,
										  0);
				if (bResult)
				{
					pDISK_EXTENT = my_vd_ex.Extents;

					if (pDISK_EXTENT->DiskNumber == devID)
					{
						sprintf(str,"%c:\\", i + 0x43);

						GetDiskFreeSpaceEx(str, NULL, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
						
						Partition_Label[ret] = i + 0x43;
						Partition_Size[ret] = (double)(i64TotalBytes / 1024.0);
						Partition_Free_Size[ret] = (double)(i64FreeBytes / 1024.0);
#ifdef DEBUG
						printf("Get partition:%c, size=%f KB\n", Partition_Label[ret], Partition_Size[ret]);
#endif
						ret++;
					}
				}
			}
			CloseHandle(hDrv);
		}
	}
	free(str);
	return ret;
}

int Get_Device_Partition_MountPoint(int index)
{
	return Partition_Label[index];
}

float Get_Device_Partition_Capacity(int index)
{
	return Partition_Size[index];
}

#endif

USHORT GetNumberOfStorage()
{
	char* ret;
	USHORT retValue;

#ifdef WIN32

#else
	ret = MEMAllocting(__func__);	
		
	GetCommandResult(ret, "lsblk -o NAME,TYPE,TRAN | grep -E 'disk.*sata|disk.*nvme' -c", NULL, 0);

	retValue = atoi(ret);

	free(ret);	
#endif

	return retValue;
}

char* GetPartitionMountPoint(char* loc, int index)
{
	char *ret, *cmd;

	ret = MEMAllocting(__func__);
#ifdef WIN32
	sprintf(ret, "%c:\\", Get_Device_Partition_MountPoint(index));

#else
	cmd = MEMAllocting(__func__);

	sprintf(cmd, "lsblk | grep part | grep %s | tail -n +%d", loc, index + 1);

	GetCommandResult(ret, cmd, " ", 6);

	free(cmd);
#endif
	return MEMReallocting(ret);
}

#ifdef WIN32
ULLONG GetCapacityInKB(char* str)
{
	int i, j;
	ULLONG ret = 0;
	j = strlen(str) - 1;
	for(i = j; i >= 0; i--)
	{
#ifdef DEBUG
		printf("str = %c, ", *(str+i));
		printf("i = %02d, ", i);
		printf("pow = %llu, ", (ULLONG)(pow(10, j-i)));
		printf("calc = %llu, ", ((ULLONG)(*(str+i) - 0x30) * (ULLONG)(pow(10, j-i))));
#endif
		ret += ((ULLONG)(*(str+i) - 0x30) * (ULLONG)(pow(10, j-i)));
#ifdef DEBUG
		printf("ret = %llu \n", ret);
#endif
	}
	return ret / 1024LLU;
}
#endif

STORAGE_INFO* lib_SysInfo_GetStorageInfo()
{
	STORAGE_INFO* ret = NULL;
	STORAGE_INFO* prev_node = NULL;
	STORAGE_PAR_INFO* prev_sub_node = NULL;
	char* LocStr = (char*)malloc(sizeof(char) * MAX_CMD_STR_LEN), *tok;
	USHORT i,j;

#ifdef WIN32
	long divider = 1;
#endif

#ifdef WIN32
	for(i = 0; i < 255; i++)
#else
	for(i = 0; i < GetNumberOfStorage(); i++)
#endif
	{
		STORAGE_INFO* current_node = (STORAGE_INFO*)malloc(sizeof(STORAGE_INFO));
		current_node->next_node = NULL;
		current_node->PARinfo = NULL; 
#ifdef WIN32
		char** tmp = GetWMIData("ROOT\\CIMV2",
							"SELECT * FROM Win32_DiskDrive",
							i,
							3,
							"Name",
							"Size",
							"Index");
		char* ttmp = *(tmp+0);
#ifdef DEBUG
		printf("Storage index : %d, Name = %s\n", atoi(*(tmp+2)), (*(tmp+0)));
#endif
		if(tmp == NULL || (*(ttmp+0) != '\\'))
		{
			if(tmp != NULL)
			{
				free(*(tmp+0));
				free(*(tmp+1));
				free(*(tmp+2));
				free(tmp);
			}
			free(current_node);
			break;
		}
#endif
		if(prev_node != NULL)
		{
			prev_node->next_node = (void*)current_node;
		}
		else
		{
			current_node->prev_node = NULL;
		}
		
#ifdef WIN32
		current_node->Index = atoi(*(tmp+2));
		current_node->Location = *(tmp+0);
		current_node->Capacity = GetCapacityInKB(*(tmp+1));
#ifdef DEBUG
		printf("Get capacity = %llu\n", current_node->Capacity);
#endif
		current_node->Capacity = current_node->Capacity / divider;
		current_node->NumberOfPartition = Get_Device_Partition_Number(current_node->Index);
		free(*(tmp+1));
		free(*(tmp+2));
		free(tmp);
		if(current_node->Capacity == 0)
		{
			prev_node->next_node = NULL;
			free(current_node->Location);
			free(current_node);
			break;
		}
#else
		current_node->Index = i;
		current_node->Location = GetStorageLocation(current_node->Index);
		strcpy(LocStr, current_node->Location);
		strtok(LocStr, "//");
		tok = strtok(NULL, "//");
		current_node->Capacity = GetStorageCapacity(tok);
		current_node->NumberOfPartition = GetNumberOfPartition(tok);
#endif
		prev_sub_node = NULL;
		for(j = 0; j < current_node->NumberOfPartition; j++)
		{
			STORAGE_PAR_INFO *current_sub_node = (STORAGE_PAR_INFO*)malloc(sizeof(STORAGE_PAR_INFO));
			current_sub_node->next_node = NULL;
			if(prev_sub_node != NULL)
			{
				prev_sub_node->next_node = (void*)current_sub_node;
			}
			else
			{
				current_sub_node->prev_node = NULL;
			}
			current_sub_node->Index = j;
#ifdef WIN32
			current_sub_node->MountPoint = GetPartitionMountPoint("",j);
			current_sub_node->Capacity = Get_Device_Partition_Capacity(j);
#else
			current_sub_node->MountPoint = GetPartitionMountPoint(tok, j);
			current_sub_node->Capacity = GetPartitionCapacity(tok, j);
#endif
			if(prev_sub_node != NULL)
			{
				current_sub_node->prev_node = (void*)prev_sub_node;
			}
			else
			{
				current_node->PARinfo = current_sub_node;
			}
			prev_sub_node = current_sub_node;
		}
		if(prev_node != NULL)
		{
			current_node->prev_node = (void*)prev_node;
		}
		else
		{
			ret = current_node;
		}
		prev_node = current_node;
	}

	free(LocStr);

	return ret;
}
