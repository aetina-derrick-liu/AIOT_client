#include <iostream>
#include <vector>
#include <pthread.h>
#include "DataBuilder.hpp"
#include "DeviceAgent.hpp"
#include "DeviceInfo.hpp"
#include "SocketAgent.hpp"
#include "DBInterface.hpp"
#include "MQTTAgent.hpp"
#include "Base64.hpp"
#include <string.h>

#include "LogAgent.hpp"

using namespace std;
extern "C"
{
	#include "libsmart.h"
}
#include "RAIDInfo.hpp"
extern char GatewayAddress[MAX_SERVICE_STR_SIZE],
			DeviceName[MAX_DEVICENAME_SIZE],
			PWD[MAX_PWD_SIZE];
			

#define INVALIDSTR "/\\. \u201C$*<>:|?"
#define MAX_PATH 1024

vector<NODE>extsensor_list;
vector<NODE>remote_list;
static SYS_INFO* ptr;

unsigned int TotalMEMCap;

pthread_mutex_t ext_mutex, remote_mutex;


char* DeleteInvaildStr(char* str)
{
	char* tok, temp[1024] = {0};
	tok = strtok(str, INVALIDSTR);
	while(tok != NULL)
	{
		if(tok != NULL)
		{
			sprintf(temp, "%s%s", temp, tok);
		}
		tok = strtok(NULL, INVALIDSTR);
	}
	strcpy(str, temp);
	return str;
}

char* GetDeviceThumbprint()
{
	char* ret = NULL;

	CheckSysInfo();

	if(ptr->mbinfo != NULL)
	{
		char MBSN[50] = {0};
		char MBProduct[50] = {0};
		char NetMac[50] = {0};

		if (ptr->mbinfo != NULL) {
			strncpy(MBSN, ptr->mbinfo->SerialNumber,strlen(ptr->mbinfo->SerialNumber));
			strncpy(MBProduct, ptr->mbinfo->ProductName,strlen(ptr->mbinfo->ProductName));
			
			DeleteInvaildStr(MBSN);
			DeleteInvaildStr(MBProduct);
		}

		if (ptr->netinfo != NULL) {
			strncpy(NetMac, ptr->netinfo->MACAddress, strlen(ptr->netinfo->MACAddress));
			DeleteInvaildStr(NetMac);
		}

		ret = (char*)malloc(sizeof(char) * (strlen(MBSN) + strlen(MBProduct) + strlen(NetMac) + 1));
		memset(ret,0,sizeof(ret));
		sprintf(ret, "%s%s%s", MBSN, MBProduct, NetMac);
	}

	return ret;
}

struct json_object* GetStaticRawData()
{
	struct json_object* ret = json_object_new_object();

	CheckSysInfo();

	if(ptr != NULL)
	{
		json_object_object_add(ret, "Dev", GetDeviceInfo());	
		json_object_object_add(ret, "Sys", GetOSInfo(ptr->osinfo));	
		json_object_object_add(ret, "CPU", GetCPUInfo(ptr->cpuinfo));
		json_object_object_add(ret, "MB", GetMBInfo(ptr->mbinfo));
		json_object_object_add(ret, "MEM", GetMEMInfo(ptr->meminfo));	
		json_object_object_add(ret, "Storage", GetStorageInfo(ptr->storinfo));	
		json_object_object_add(ret, "Net", GetNETInfo(ptr->netinfo));
		json_object_object_add(ret, "Ext", GetExternalSensorList());
		json_object_object_add(ret, "Remote", GetRemoteDeviceList());
		json_object_object_add(ret, "GPU", GetGPUInfo(ptr->gpuinfo));

	}
	return ret;
}

struct json_object* GetDynamicRawData()
{
	struct json_object* ret = json_object_new_object();
	struct json_object* mem = json_object_new_object();
	CheckRawDataCount();

	CheckSysInfo();

	SYS_DYNAMIC_INFO* dyptr	= lib_SysInfo_GetSysDyInfo();
	if(dyptr != NULL)
	{
		json_object_object_add(ret, "Dev", json_object_new_string(DeviceName));	
		json_object_object_add(ret, "CPU", GetCPUDynamicInfo(dyptr->cpuinfo));
		json_object_object_add(ret, "mbProbe", GetMBDynamicInfo(dyptr->mbinfo));
		json_object_object_add(mem, "temp", json_object_new_double(dyptr->memtemp));
#ifdef WIN32
		json_object_object_add(mem, "memUsed", json_object_new_int(TotalMEMCap*dyptr->memless/100));
#else
		json_object_object_add(mem, "memUsed", json_object_new_int(TotalMEMCap-dyptr->memless));
#endif
		json_object_object_add(ret, "MEM", mem);
		json_object_object_add(ret, "Storage", GetStorageDynamicInfo(ptr->storinfo));
#ifdef WIN32
		json_object_object_add(ret, "EAPI", GetEApiDynamicInfo(dyptr->eapiinfo));
#endif
		pthread_mutex_lock(&ext_mutex);
		json_object_object_add(ret, "Ext", GetExternalSensorValue());
		json_object_object_add(ret,"GPU",GetGPUDynamicInfo(dyptr->gpuinfo));
		pthread_mutex_unlock(&ext_mutex);
		GetLocalDynamicRawData(dyptr);
		if(dyptr != NULL)
			lib_SysInfo_free_DyInfo(dyptr);
	}

	return ret;
}
/***************************************************/
FILE* wfopen(const wchar_t* filename, const wchar_t* mode)
{
    char fn[MAX_PATH] = {0};
    char m[MAX_PATH] = {0};
    wcstombs(fn, filename, MAX_PATH);
    wcstombs(m, mode, MAX_PATH);
    return fopen(fn, m);
}

/***************************************************/

struct json_object* GetScreenshot(wchar_t* ImgName)
{
	unsigned int size;
	struct json_object* ret = json_object_new_object();
	FILE *hFileTmp;
	hFileTmp=wfopen(ImgName, L"r");
	if(hFileTmp==NULL)
	{
		json_object_object_add(ret, "Img", json_object_new_string("") );
		fclose(hFileTmp);
		return ret;
	}
	fseek(hFileTmp, 0L, SEEK_END); 
	fseek(hFileTmp, 0L, SEEK_SET);
	char* Image=(char*)malloc(sizeof(char)*35000);
	char* Image_encode=(char*)malloc(sizeof(char)*65536);
	memset(Image,0,35000);
	memset(Image_encode,0,65536);
	fread(Image, 1, 35000, hFileTmp);
	fclose(hFileTmp);
	base64_encode((unsigned char*)Image,
		  (unsigned char*)Image_encode,
		  35000,
		  0);
	json_object_object_add(ret, "Dev", json_object_new_string(DeviceName));
	json_object_object_add(ret, "Img", json_object_new_string(Image_encode));
	free(Image);
	free(Image_encode);
	return ret;

}

struct json_object* GetExternalSensorList()
{
	pthread_mutex_lock(&ext_mutex);
	struct json_object* ret = json_object_new_object();
	unsigned char index = 0;
	char* IndexString = (char*)malloc(sizeof(char) * 5);
	for(int i=0;i < extsensor_list.size();i++)
	{
		struct json_object* sub = json_object_new_object();
		json_object_object_add(sub, "Name", json_object_new_string(strdup(extsensor_list[i].Name.c_str())));
		json_object_object_add(sub, "Unit", json_object_new_string(strdup(extsensor_list[i].Unit.c_str())));
		json_object_object_add(sub, "Type", json_object_new_int(extsensor_list[i].type));
		memset(IndexString, 0, 5);
		sprintf(IndexString, "%d", index++);
		json_object_object_add(ret, IndexString, sub);
	}
	free(IndexString);
	pthread_mutex_unlock(&ext_mutex);
	return ret;
}

struct json_object* GetExternalSensorValue()
{
	struct json_object* ret = json_object_new_object();
	unsigned char index = 0;
	char* IndexString = (char*)malloc(sizeof(char) * 5);
	for(int i=0;i<extsensor_list.size();i++)
	{
		float value = SA_GetExtSensorValue(strdup(extsensor_list[i].Name.c_str()));
		memset(IndexString, 0, 5);
		sprintf(IndexString, "%d", index++);
		json_object_object_add(ret, IndexString, json_object_new_double(value));
		extsensor_list[i].last_value=value;
	}
	free(IndexString);
	return ret;
}

struct json_object* GetRemoteDeviceList()
{
	pthread_mutex_lock(&remote_mutex);
	struct json_object* ret = json_object_new_object();
	unsigned char index = 0;
	char* IndexString = (char*)malloc(sizeof(char) * 5);
	for(int i=0;i<remote_list.size();i++)
	{
		struct json_object* sub = json_object_new_object();
		json_object_object_add(sub, "Name", json_object_new_string(strdup(remote_list[i].Name.c_str())));
		json_object_object_add(sub, "Unit", json_object_new_string(strdup(remote_list[i].Unit.c_str())));
		json_object_object_add(sub, "Type", json_object_new_int(remote_list[i].type));
		memset(IndexString, 0, 5);
		sprintf(IndexString, "%d", index++);
		json_object_object_add(ret, IndexString, sub);
	}
	free(IndexString);
	pthread_mutex_unlock(&remote_mutex);
	return ret;
}
int AddExternalSensor(char* name, char* unit, unsigned char type)
{	
	pthread_mutex_lock(&ext_mutex);
	NODE node;
	node.Name=string(name);
	node.Unit=string(unit);
	node.type=type;
	extsensor_list.push_back(node);
	pthread_mutex_unlock(&ext_mutex);

	return 0;
}

int AddRemoteDevice(char* name, char* unit, unsigned char type)
{
	pthread_mutex_lock(&remote_mutex);
	NODE node;
	node.Name=string(name);
	node.Unit=string(unit);
	node.type=type;
	int flag=0;
	for(auto it = remote_list.begin(); it != remote_list.end(); ++it)
	{
		if(it->Name==string(name))
		{
			flag=1;
			break;
		}
	}
	if(flag==0)
	{
		remote_list.push_back(node);
	}
	pthread_mutex_unlock(&remote_mutex);
	return 0;
}

int RemoveExternalSensor(char* name)
{
	pthread_mutex_lock(&ext_mutex);
	char modified = 0;
	string name_str=string(name);
	for(int i=0;i<extsensor_list.size();i++)
	{
		if((extsensor_list[i].Name).compare(name_str)==0)
		{
			modified=1;
			extsensor_list.erase(extsensor_list.begin()+i);
		}
	}
	
	pthread_mutex_unlock(&ext_mutex);
	if(modified)
	{
		return 0;
	}
	return -1;
}

int RemoveRemoteDevice(char* name)
{
	pthread_mutex_lock(&remote_mutex);
	char modified = 0;
	string name_str=string(name);
	for(int i=0;i<remote_list.size();i++)
	{
		if((remote_list[i].Name).compare(name_str)==0)
		{
			modified=1;
			remote_list.erase(remote_list.begin()+i);
		}
	}
	pthread_mutex_unlock(&remote_mutex);
	if(modified)
	{
		return 0;
	}
	return -1;
}

void ClearExternalInformation()
{
	pthread_mutex_lock(&ext_mutex);
	extsensor_list.clear();
	remote_list.clear();
	pthread_mutex_unlock(&ext_mutex);
}

// int CheckInnodisk()
// {
// 	int StorInno = -1;
// 	int RaidEXPSInno = 0;
// 	int ret = 0	;

// 	CheckSysInfo();
// 	StorInno = CheckInno(ptr->storinfo);
// // #ifdef WIN32
// // 	RaidEXPSInno = CheckEXPSInno();
// // #endif
// 	if(StorInno >= 1/*||RaidEXPSInno == 1*/)	ret = 1;
// 	return ret;
// }

void CheckDataGroup()
{
	InsertDataGroup((char*)"System");
	InsertDataGroup((char*)"CPU");
	InsertDataGroup((char*)"Motherboard");
	InsertDataGroup((char*)"Memory");
	InsertDataGroup((char*)"Storage");
	InsertDataGroup((char*)"NET");
	InsertDataGroup((char*)"External");
}

void CheckData()
{
	//InsertDataDefine(char* Name, char* Location, int Numberical, int GroupId);
	//System information
	InsertDataDefine((char*)"OS name", (char*)"{\"Static\":{\"Sys\":{\"OS\":0}}}", 0, 1);
	InsertDataDefine((char*)"OS version", (char*)"{\"Static\":{\"Sys\":{\"OSVer\":0}}}", 0, 1);
	InsertDataDefine((char*)"OS architecture", (char*)"{\"Static\":{\"Sys\":{\"OSArch\":0}}}", 0, 1);
	InsertDataDefine((char*)"Computer name", (char*)"{\"Static\":{\"Sys\":{\"Name\":0}}}", 0, 1);
	InsertDataDefine((char*)"Device longitude", (char*)"{\"Static\":{\"Sys\":{\"Longitude\":0}}}", 1, 1);
	InsertDataDefine((char*)"Device latitude", (char*)"{\"Static\":{\"Sys\":{\"Latitude\":0}}}", 1, 1);

	//CPU information
	InsertDataDefine((char*)"CPU manufacturer", (char*)"{\"Static\":{\"CPU\":{\"Manu\":0}}}", 0, 2);
	InsertDataDefine((char*)"CPU name", (char*)"{\"Static\":{\"CPU\":{\"Name\":0}}}", 0, 2);
	InsertDataDefine((char*)"Number of CPU core", (char*)"{\"Static\":{\"CPU\":{\"Numofcore\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU L2 cache size", (char*)"{\"Static\":{\"CPU\":{\"L2\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU L3 cache size",(char*) "{\"Static\":{\"CPU\":{\"L3\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU frequency", (char*)"{\"Dynamic\":{\"CPU\":{\"Freq\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU usage", (char*)"{\"Dynamic\":{\"CPU\":{\"Usage\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU fan rotating speed", (char*)"{\"Dynamic\":{\"CPU\":{\"FanRPM\":0}}}", 1, 2);
	InsertDataDefine((char*)"CPU each core frequency", (char*)"{\"Dynamic\":{\"CPU\":{\"0\":{\"Freq\":0}}}}", 1, 2);
	InsertDataDefine((char*)"CPU each core usage",(char*) "{\"Dynamic\":{\"CPU\":{\"0\":{\"Usage\":0}}}}", 1, 2);
	InsertDataDefine((char*)"CPU each core temperature", (char*)"{\"Dynamic\":{\"CPU\":{\"0\":{\"Temp\":0}}}}", 1, 2);
	InsertDataDefine((char*)"CPU each core voltage", (char*)"{\"Dynamic\":{\"CPU\":{\"0\":{\"V\":0}}}}", 1, 2);

	//Motherboard information
	InsertDataDefine((char*)"Motherboard manufacturer",(char*) "{\"Static\":{\"MB\":{\"Manu\":0}}}", 0, 3);
	InsertDataDefine((char*)"Motherboard product",(char*) "{\"Static\":{\"MB\":{\"Product\":0}}}", 0, 3);
	InsertDataDefine((char*)"Motherboard serial number", (char*)"{\"Static\":{\"MB\":{\"SN\":0}}}", 0, 3);
	InsertDataDefine((char*)"BIOS manufacturer",(char*) "{\"Static\":{\"MB\":{\"BIOSManu\":0}}}", 0, 3);
	InsertDataDefine((char*)"BIOS version",(char*) "{\"Static\":{\"MB\":{\"BIOSVer\":0}}}", 0, 3);

	//Memory information
	InsertDataDefine((char*)"Memory total capacity", (char*)"{\"Static\":{\"MEM\":{\"Cap\":0}}}", 1, 4);
	InsertDataDefine((char*)"Memory manufacturer",(char*) "{\"Static\":{\"MEM\":{\"0\":{\"Manu\":0}}}}", 0, 4);
	InsertDataDefine((char*)"Memory location", (char*)"{\"Static\":{\"MEM\":{\"0\":{\"Loc\":0}}}}", 0, 4);
	InsertDataDefine((char*)"Memory capacity", (char*)"{\"Static\":{\"MEM\":{\"0\":{\"Cap\":0}}}}", 1, 4);
	InsertDataDefine((char*)"Memory frequency",(char*) "{\"Static\":{\"MEM\":{\"0\":{\"Freq\":0}}}}", 1, 4);
	InsertDataDefine((char*)"Memory voltage", (char*)"{\"Static\":{\"MEM\":{\"0\":{\"V\":0}}}}", 1, 4);
	InsertDataDefine((char*)"Memory unused size", (char*)"{\"Dynamic\":{\"memLess\":0}}", 1, 4);

	//Storage information
	InsertDataDefine((char*)"Storage model", (char*)"{\"Static\":{\"Storage\":[{\"Model\":0}]}}", 0, 5);
	InsertDataDefine((char*)"Storage serial number", (char*)"{\"Static\":{\"Storage\":[{\"SN\":0}]}}", 0, 5);
	InsertDataDefine((char*)"Storage firmware version",(char*) "{\"Static\":{\"Storage\":[{\"FWVer\":0}]}}", 0, 5);
	InsertDataDefine((char*)"Storage total capacity", (char*)"{\"Static\":{\"Storage\":[{\"Par\":{\"TotalCap\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Number of storage partition",(char*) "{\"Static\":{\"Storage\":[{\"Par\":{\"NumofPar\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage partition location", (char*)"{\"Static\":{\"Storage\":[{\"Par\":{\"ParInfo\":[{\"MountAt\":0}]}}]}}", 0, 5);
	InsertDataDefine((char*)"Storage partition capacity",(char*) "{\"Static\":{\"Storage\":[{\"Par\":{\"ParInfo\":[{\"Capacity\":0}]}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage power on hours", (char*)"{\"Dynamic\":{\"Storage\":[{\"smart\":{\"9\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage power on cycles", (char*)"{\"Dynamic\":{\"Storage\":[{\"smart\":{\"12\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage average erase count", (char*)"{\"Dynamic\":{\"Storage\":[{\"smart\":{\"167\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage temperature", (char*)"{\"Dynamic\":{\"Storage\":[{\"smart\":{\"194\":0}}]}}", 1, 5);
	InsertDataDefine((char*)"Storage health", (char*)"{\"Dynamic\":{\"Storage\":[{\"Health\":0}]}}", 1, 5);
	InsertDataDefine((char*)"Storage PECycle", (char*)"{\"Dynamic\":{\"Storage\":[{\"PECycle\":0}]}}", 1, 5);
	InsertDataDefine((char*)"Storage iAnalyzer data", (char*)"{\"Dynamic\":{\"Storage\":[{\"iAnalyzer\":{\"Enable\":1}}]}}", 0, 5);
	InsertDataDefine((char*)"Storage estimation lifespan", (char*)"{\"StorageAnalyzer\":{\"Lifespan\":[{\"data\":0}]}}", 1, 5);

	//Network card information
	InsertDataDefine((char*)"Network card name", (char*)"{\"Static\":{\"Net\":[{\"Name\":0}]}}", 0, 6);
	InsertDataDefine((char*)"Network card type", (char*)"{\"Static\":{\"Net\":[{\"Type\":0}]}}", 0, 6);
	InsertDataDefine((char*)"Network card MAC address", (char*)"{\"Static\":{\"Net\":[{\"MAC\":0}]}}", 0, 6);
	InsertDataDefine((char*)"Network card IP address", (char*)"{\"Static\":{\"Net\":[{\"IPAddr\":0}]}}", 0, 6);
	InsertDataDefine((char*)"Network card IPv6 address", (char*)"{\"Static\":{\"Net\":[{\"IPv6\":0}]}}", 0, 6);
	InsertDataDefine((char*)"Network card netmask", (char*)"{\"Static\":{\"Net\":[{\"Netmask\":0}]}}", 0, 6);
	//InsertThreshold(int DataId, char enable, int Func, float value);
	//Threshold
	InsertThreshold(41, 1, 2, 50);
	InsertThreshold(45, 1, 1, 150);
	InsertThreshold(43, 1, 2, 1000);
}

void GetLocalStorageInfo()
{
	CheckSysInfo();
	if(ptr != NULL)
	{
		GetLocalStor(ptr->storinfo);
		TotalMEMCap = ptr->meminfo->Capacity;
	}
}
void GetLocalDynamicRawData(void* local_ptr)
{
	float current, total;
	int lastRawId;
	SYS_DYNAMIC_INFO* dyptr;
	STORAGE_INFO* node;
	int temperature, avgEraseCount, lifespan;

	CheckRawDataCount();


	CheckSysInfo();

	if(local_ptr == NULL)
	{
		dyptr	= lib_SysInfo_GetSysDyInfo();
	}
	else
	{
		dyptr = (SYS_DYNAMIC_INFO*)local_ptr;
	}

	if(dyptr != NULL)
	{
		if(CheckStorChange())
		{
			SendStaticRawData();
		}
		GetLocalStor(dyptr->storinfo);
		current = TotalMEMCap - dyptr->memless;
		total = TotalMEMCap;

#ifdef WIN32
		lastRawId = InsertRawData(dyptr->cpuinfo->Usage, dyptr->memless);
#else
		lastRawId = InsertRawData(dyptr->cpuinfo->Usage, (current / total) * 100);
#endif

		node = dyptr->storinfo;
		
		while(node != NULL)
		{
			int ret1,j;
			char* SNStr = (char*)malloc(sizeof(char) * 50);
			char* time=NULL;
			DISK_INFO disk_info;
			ATTRIB_INFO* attrib_info=NULL;
			IDENTIFY_INFO* id_info;
			ANALYZER_INFO *analyzer_info;
			SMART_INFO *ismart_info;
			LIFESPAN_INFO *life;
			temperature = 0;
			avgEraseCount = 0;

#ifdef WIN32
			set_analyzer(node->Index, 1);
			ret1 = get_dev_info(node->Index, &disk_info);

			attrib_info = disk_info.smart.attrib;
			id_info = &disk_info.id;
			analyzer_info = &disk_info.analyzer;
#else
			set_analyzer(node->Location, 1);
			ret1 = get_dev_info(node->Location, &disk_info);

			attrib_info = disk_info.smart.attrib;
			id_info = &disk_info.id;
			analyzer_info = &disk_info.analyzer;
#endif
			memset(SNStr, 0, 50);
			strncpy(SNStr, id_info->SerialNum, SN_LEN);
			SNStr = lib_SysInfo_TrimWhiteSpace(SNStr);
			avgEraseCount = disk_info.dev.AvgErase;
			temperature = disk_info.dev.Temperature;

			InsertStorageRawData(SNStr, lastRawId, disk_info.dev.Health, temperature, avgEraseCount);
			time = GetStorageDataLastModifyDate(SNStr);
			if(time == NULL)
			{
				free(SNStr);
			}
			else
			{
				int YY,MM,DD,hh,mm,ss;
				int lifespan;
				LIFESPAN_DATA* stor_data;
				sscanf(time,"%d-%d-%d %d:%d:%d",&YY,&MM,&DD,&hh,&mm,&ss);
				life=(LIFESPAN_INFO*)malloc(sizeof(LIFESPAN_INFO));
				life->yy=YY;
				life->mm=MM;
				life->dd=DD;
				life->health=GetStorageInitHealth(SNStr);
				strcpy(life->sn,SNStr);
#ifdef WIN32
				lifespan=get_lifespan(node->Index,life);
#else
				lifespan=get_lifespan(node->Location,life);
#endif
				stor_data = GetStorageParameter(SNStr);
				if(lifespan >  0)
				{
					InsertLifespanData(SNStr, stor_data, lifespan);
				}
				if(stor_data->PECycle > 0)
				{
					CheckStorageThreshold(SNStr, temperature, avgEraseCount, lifespan);
				}
				free(SNStr);
				free(time);
				free(life);
			}
			node = (STORAGE_INFO*)node->next_node;
		}
		for(int i=0;i<extsensor_list.size();i++)
		{
			InsertExtDataRaw(lastRawId,&(extsensor_list[i].Name)[0u],extsensor_list[i].last_value);
		}
		if (local_ptr == NULL)
		{
			lib_SysInfo_free_DyInfo(dyptr);
			local_ptr = NULL;
		}
	}
// #ifdef WIN32
// 	if(CheckEXPSInno())	GetEXPS_StorLife(lastRawId);
// #endif
}

void GetSysInfo()
{
	if(ptr != NULL)
	{
		lib_SysInfo_free(ptr);
	}
	ptr = lib_SysInfo_GetSysInfo();
}

void CheckSysInfo()
{
	if(ptr == NULL)
	{
		ptr = lib_SysInfo_GetSysInfo();
	}
}

void FreeSysInfo()
{
	if(ptr != NULL)
	{
		lib_SysInfo_free(ptr);
	}
}
