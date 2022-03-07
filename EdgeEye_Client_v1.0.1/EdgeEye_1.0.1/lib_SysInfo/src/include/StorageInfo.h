#ifndef __STORAGEINFO_H__
#define __STORAGEINFO_H__

#include "lib_SysInfo.h"

#ifndef WIN32
USHORT GetNumberOfStorage();
char* GetStorageLocation(USHORT index);
UINT GetStorageCapacity(char* loc);
USHORT GetNumberOfPartition(char* loc);
UINT GetPartitionCapacity(char* loc, int index);
#endif
char* GetPartitionMountPoint(char* loc, int index);
STORAGE_INFO* lib_SysInfo_GetStorageInfo();

#endif
