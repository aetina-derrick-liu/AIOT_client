#include <string.h>
#include <iostream>
#include <unordered_set>
#include "DeviceInfo.hpp"
extern "C"
{
	#include "libsmart.h"
}
#include "DBInterface.hpp"
#include "RAIDInfo.hpp"
#include "JsonParser.hpp"

double Longitude;
double Latitude;

using namespace std;
static int StorNum = 0;
unordered_set <string> StorList;//SN

int isUTF8(const char *data, size_t size)
{
    const unsigned char *str = (unsigned char*)data;
    const unsigned char *end = str + size;
    unsigned char byte;
    unsigned int code_length, i;
    uint32_t ch;
    while (str != end) {
        byte = *str;
        if (byte <= 0x7F) {
            /* 1 byte sequence: U+0000..U+007F */
            str += 1;
            continue;
        }

        if (0xC2 <= byte && byte <= 0xDF)
            /* 0b110xxxxx: 2 bytes sequence */
            code_length = 2;
        else if (0xE0 <= byte && byte <= 0xEF)
            /* 0b1110xxxx: 3 bytes sequence */
            code_length = 3;
        else if (0xF0 <= byte && byte <= 0xF4)
            /* 0b11110xxx: 4 bytes sequence */
            code_length = 4;
        else {
            /* invalid first byte of a multibyte character */
            return 0;
        }

        if (str + (code_length - 1) >= end) {
            /* truncated string or invalid byte sequence */
            return 0;
        }

        /* Check continuation bytes: bit 7 should be set, bit 6 should be
         * unset (b10xxxxxx). */
        for (i=1; i < code_length; i++) {
            if ((str[i] & 0xC0) != 0x80)
                return 0;
        }

        if (code_length == 2) {
            /* 2 bytes sequence: U+0080..U+07FF */
            ch = ((str[0] & 0x1f) << 6) + (str[1] & 0x3f);
            /* str[0] >= 0xC2, so ch >= 0x0080.
               str[0] <= 0xDF, (str[1] & 0x3f) <= 0x3f, so ch <= 0x07ff */
        } else if (code_length == 3) {
            /* 3 bytes sequence: U+0800..U+FFFF */
            ch = ((str[0] & 0x0f) << 12) + ((str[1] & 0x3f) << 6) +
                  (str[2] & 0x3f);
            /* (0xff & 0x0f) << 12 | (0xff & 0x3f) << 6 | (0xff & 0x3f) = 0xffff,
               so ch <= 0xffff */
            if (ch < 0x0800)
                return 0;

            /* surrogates (U+D800-U+DFFF) are invalid in UTF-8:
               test if (0xD800 <= ch && ch <= 0xDFFF) */
            if ((ch >> 11) == 0x1b)
                return 0;
        } else if (code_length == 4) {
            /* 4 bytes sequence: U+10000..U+10FFFF */
            ch = ((str[0] & 0x07) << 18) + ((str[1] & 0x3f) << 12) +
                 ((str[2] & 0x3f) << 6) + (str[3] & 0x3f);
            if ((ch < 0x10000) || (0x10FFFF < ch))
                return 0;
        }
        str += code_length;
    }
    return 1;
}
struct json_object* GetOSInfo(OS_INFO* ptr)
{
	struct json_object* ret = json_object_new_object();

	if(isUTF8(ptr->Name,strlen(ptr->Name)))
	{
		json_object_object_add(ret, "OS", json_object_new_string(ptr->Name));
	}
	else
	{
		json_object_object_add(ret, "OS", json_object_new_string(""));
	}
	if(isUTF8(ptr->Version,strlen(ptr->Version)))
	{
		json_object_object_add(ret, "OSVer", json_object_new_string(ptr->Version));
	}
	else
	{
		json_object_object_add(ret, "OSVer", json_object_new_string(""));
	}
	if(isUTF8(ptr->Arch,strlen(ptr->Arch)))
	{
		json_object_object_add(ret, "OSArch", json_object_new_string(ptr->Arch));
	}
	else
	{
		json_object_object_add(ret, "OSArch", json_object_new_string(""));
	}
	if(isUTF8(ptr->HostName,strlen(ptr->HostName)))
	{
		json_object_object_add(ret, "Name", json_object_new_string(ptr->HostName));
	}
	else
	{
		json_object_object_add(ret, "Name", json_object_new_string(""));
	}

	json_object_object_add(ret, "Longitude", json_object_new_double(Longitude));
	json_object_object_add(ret, "Latitude", json_object_new_double(Latitude));
	return ret;
}

struct json_object* GetCPUInfo(CPU_INFO* ptr)
{
	struct json_object* ret = json_object_new_object();
	json_object_object_add(ret, "Manu", json_object_new_string(ptr->Manufacturer));
	json_object_object_add(ret, "Name", json_object_new_string(ptr->Name));
	json_object_object_add(ret, "Numofcore", json_object_new_int(ptr->NumberOfCore));
	json_object_object_add(ret, "L2", json_object_new_int(ptr->L2Size));
	json_object_object_add(ret, "L3", json_object_new_int(ptr->L3Size));
	return ret;
}

struct json_object* GetMBInfo(MB_INFO* ptr)
{
	struct json_object* ret = json_object_new_object();
#ifndef __aarch64__
	
#else
	json_object_object_add(ret, "Manu", json_object_new_string(ptr->Manufacturer));
	json_object_object_add(ret, "Product", json_object_new_string(ptr->ProductName));
	json_object_object_add(ret, "SN", json_object_new_string(ptr->SerialNumber));
	json_object_object_add(ret, "BIOSManu", json_object_new_string(ptr->BIOSinfo->Manufacturer));
	json_object_object_add(ret, "BIOSVer", json_object_new_string(ptr->BIOSinfo->Version));
	json_object_object_add(ret, "mbTemp", json_object_new_object());
#endif

	return ret;
}

struct json_object* GetMEMInfo(MEM_INFO* ptr)
{
	struct json_object* ret = json_object_new_object();
	struct json_object* mem = json_object_new_object();
	MEMSLOT_INFO* node = ptr->MEMSlot;
	json_object_object_add(ret, "Cap", json_object_new_int(ptr->Capacity));
	while(node != NULL)
	{
		if(node->Capacity > 0)
		{
			struct json_object* sub = json_object_new_object();
			json_object_object_add(sub, "Type", json_object_new_string(node->Type));
			json_object_object_add(sub, "Manu", json_object_new_string(node->Manufacturer));
			json_object_object_add(sub, "Cap", json_object_new_int(node->Capacity));
			json_object_object_add(sub, "SN", json_object_new_string(node->SN));

#ifdef WIN32
			json_object_object_add(sub, "PN", json_object_new_string(node->PN));
			json_object_object_add(sub, "Date", json_object_new_string(node->Date));
			json_object_object_add(sub, "DIMMType", json_object_new_string(node->DIMMType));
			json_object_object_add(sub, "OPTemp", json_object_new_string(node->OPTemp));
			json_object_object_add(sub, "IC_Cfg", json_object_new_string(node->IC_Cfg));
			json_object_object_add(sub, "IC_Brand", json_object_new_string(node->IC_Brand));
			json_object_object_add(sub, "Therm", json_object_new_string(node->Thermal));
			json_object_object_add(sub, "CAS_Ltc", json_object_new_double((double)node->CAS_Ltc));
			json_object_object_add(sub, "Rate", json_object_new_int(node->Rate));
			if(node->feature)
			{
				struct json_object* ftr = json_object_new_object();
				json_object_object_add(ftr, "sICGrade", json_object_new_string(node->feature->sICGrade));
				json_object_object_add(ftr, "AntiSul", json_object_new_int(node->feature->AntiSul));
				json_object_object_add(ftr, "30GF", json_object_new_boolean(node->feature->b30GoldenFing));
				json_object_object_add(ftr, "45GF", json_object_new_boolean(node->feature->b45GoldenFing));
				json_object_object_add(ftr, "bWP", json_object_new_boolean(node->feature->bWP));
				json_object_object_add(sub, "Feature", json_object_get(ftr));
			}
#else
			json_object_object_add(sub, "Loc", json_object_new_string(node->Location));
			json_object_object_add(sub, "Date", json_object_new_string(""));
			json_object_object_add(sub, "PN", json_object_new_string(node->PN));
			json_object_object_add(sub, "Rate", json_object_new_int(node->Rate));
#endif

			char* indexstr = (char*)malloc(sizeof(char) * 8);
			sprintf(indexstr, "%d", node->Index);
			json_object_object_add(mem, indexstr, json_object_get(sub));
			free(indexstr);
			json_object_put(sub);
		}
		node = (MEMSLOT_INFO*)node->next_node;
	}
	json_object_object_add(ret, "Slot", mem);
	return ret;
}

struct json_object* GetStorageInfo(STORAGE_INFO* ptr)
{
	struct json_object* ret = json_object_new_array();
	STORAGE_INFO* node = ptr;
	STORAGE_PAR_INFO* subnode = NULL;
	while(node != NULL)
	{
		DISK_INFO disk_info;
		IDENTIFY_INFO *id_info;
		char* tmp = (char*)malloc(sizeof(char) * 50);
		struct json_object *sub, *sub2, *sub3arr;
		sub = json_object_new_object();
		sub2 = json_object_new_object();
		sub3arr = json_object_new_array();

#ifdef WIN32
		set_analyzer(node->Index, 1);
		get_dev_info(node->Index, &disk_info);
		id_info = &disk_info.id;
#else
		set_analyzer(node->Location, 1);
		get_dev_info(node->Location, &disk_info);
		id_info = &disk_info.id;
#endif

		json_object_object_add(sub, "Index", json_object_new_int(node->Index));
		
		memset(tmp, 0, 50);
		strncpy(tmp, id_info->ModelName, MN_LEN);
		tmp = lib_SysInfo_TrimWhiteSpace(tmp);
		json_object_object_add(sub, "Model", json_object_new_string(tmp));

		memset(tmp, 0, 50);
		strncpy(tmp, id_info->SerialNum, SN_LEN);
		tmp = lib_SysInfo_TrimWhiteSpace(tmp);
		json_object_object_add(sub, "SN", json_object_new_string(tmp));

		memset(tmp, 0, 50);
		strncpy(tmp, id_info->FWVer, FW_LEN);
		tmp = lib_SysInfo_TrimWhiteSpace(tmp);
		json_object_object_add(sub, "FWVer", json_object_new_string(tmp));

		free(tmp);

		json_object_object_add(sub2, "TotalCap", json_object_new_int(node->Capacity));

		json_object_object_add(sub2, "NumofPar", json_object_new_int(node->NumberOfPartition));

		subnode = node->PARinfo;

		if(node->NumberOfPartition > 0)
		{
			while(subnode != NULL)
			{
				struct json_object *sub4 = json_object_new_object();
			
				json_object_object_add(sub4, "MountAt", json_object_new_string(subnode->MountPoint));
				
				json_object_object_add(sub4, "Capacity", json_object_new_int(subnode->Capacity));
				
				json_object_array_add(sub3arr, sub4);
			
				subnode = (STORAGE_PAR_INFO*)subnode->next_node;
			}
		}

		json_object_object_add(sub2, "ParInfo", sub3arr);

		json_object_object_add(sub, "Par", sub2);

		json_object_array_add(ret, sub);
		node = (STORAGE_INFO*)node->next_node;
	}
	return ret;
}

struct json_object* GetNETInfo(NET_INFO* ptr)
{
	struct json_object* ret = json_object_new_array();
	NET_INFO* node = ptr;
	while(node != NULL)
	{
		struct json_object* sub = json_object_new_object();
		json_object_object_add(sub, "Name", json_object_new_string(node->Name));
		json_object_object_add(sub, "Type", json_object_new_string(node->Type));
		json_object_object_add(sub, "MAC", json_object_new_string(node->MACAddress));
		json_object_object_add(sub, "IPv6", json_object_new_string(node->IPv6Address));
		json_object_object_add(sub, "IPaddr", json_object_new_string(node->IPv4Address));
		json_object_object_add(sub, "Netmask", json_object_new_string(node->Netmask));
		json_object_array_add(ret, sub);
		node = (NET_INFO*)node->next_node;
	}
	return ret;
}

struct json_object* GetCPUDynamicInfo(CPU_DYNAMIC_INFO* ptr)
{	
	int index;
	char* indexstr = (char*)malloc(sizeof(char) * 5);
	struct json_object* ret = json_object_new_object();
	json_object_object_add(ret, "Freq", json_object_new_int(ptr->Frequency));
	json_object_object_add(ret, "Usage", json_object_new_double(ptr->Usage));
	json_object_object_add(ret, "FanRPM", json_object_new_int(ptr->FanRPM));
	CPU_CORE_INFO* node = ptr->core;
	index = 0;
	while(node != NULL)
	{
		struct json_object* sub = json_object_new_object();
		json_object_object_add(sub, "Freq", json_object_new_int(node->Frequency));
		json_object_object_add(sub, "Usage", json_object_new_double(node->Usage));
		json_object_object_add(sub, "Temp", json_object_new_int(node->Temperature));
		json_object_object_add(sub, "V", json_object_new_double(node->Voltage));
		memset(indexstr, 0, 5);
		sprintf(indexstr, "%u", index++);
		json_object_object_add(ret, indexstr, sub);
		node = (CPU_CORE_INFO*)node->next_node;
	}
	free(indexstr);
	return ret;
}

struct json_object* GetMBDynamicInfo(MB_DYNAMIC_INFO* ptr)
{

	return json_object_new_object();
}

struct json_object* GetStorageDynamicInfo(STORAGE_INFO* ptr)
{
	struct json_object* ret = json_object_new_array();
	STORAGE_INFO* node = ptr;
	int powerOnHours, powerOnCycles, temperature, avgEraseCount, lifespan;
	while(node != NULL)
	{
		struct json_object *jobj, *jsmart, *janalyzer;
		int ret1,j;
		char* IDStr = (char*)malloc(sizeof(char) * 10);
		char* SNStr = (char*)malloc(sizeof(char) * 50);
		DISK_INFO disk_info;
		ATTRIB_INFO* attrib_info;
		IDENTIFY_INFO* id_info;
		ANALYZER_INFO *analyzer_info;
		SMART_INFO *ismart_info;
		powerOnHours = 0;
		powerOnCycles = 0;
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
		jobj = json_object_new_object();
		jsmart = json_object_new_object();
		janalyzer = json_object_new_object();		

		powerOnHours = *(&disk_info.dev.PowerOnHours);
		powerOnCycles = *(&disk_info.dev.PowerOnCycles);
		avgEraseCount = *(&disk_info.dev.AvgErase);
		temperature = *(&disk_info.dev.Temperature);

		json_object_object_add(jsmart, "9", json_object_new_int(powerOnHours));
		json_object_object_add(jsmart, "12", json_object_new_int(powerOnCycles));
		json_object_object_add(jsmart, "167", json_object_new_int(avgEraseCount));
		json_object_object_add(jsmart, "194", json_object_new_int(temperature));

		json_object_object_add(jobj, "Index", json_object_new_int(node->Index));
		json_object_object_add(jobj, "SN", json_object_new_string(SNStr));
		free(SNStr);
		json_object_object_add(jobj, "smart", jsmart);
		json_object_object_add(jobj, "Health", json_object_new_double(disk_info.dev.Health));
		json_object_object_add(jobj, "PECycle", json_object_new_int((unsigned int)disk_info.dev.PECycle));

#ifdef WIN32
		if(disk_info.id.feature.Analyzer)
#else
		if(disk_info.id.feature.Analyzer)
#endif
		{
			struct json_object *src, *rrc, *swc, *rwc;
			src = json_object_new_object();
			rrc = json_object_new_object();
			swc = json_object_new_object();
			rwc = json_object_new_object();

			if(analyzer_info->ver == 0)
			{
				json_object_object_add(janalyzer, "Enable", json_object_new_int(1));
			}
			else
			{
				json_object_object_add(janalyzer, "Enable", json_object_new_int(2));
			}
			json_object_object_add(janalyzer, "SRC", json_object_new_int(analyzer_info->TR_CNT[0]));
			json_object_object_add(janalyzer, "RRC", json_object_new_int(analyzer_info->TR_CNT[1]));
			json_object_object_add(janalyzer, "SWC", json_object_new_int(analyzer_info->TW_CNT[0]));
			json_object_object_add(janalyzer, "RWC", json_object_new_int(analyzer_info->TW_CNT[1]));
			
			for(j = 0; j < analyzer_info->cnt_sequential; j++)
			{		
				memset(IDStr, 0, 10);
				sprintf(IDStr, "%d", j);
				json_object_object_add(src, IDStr, json_object_new_int(analyzer_info->SR_CNT[j]));
				json_object_object_add(swc, IDStr, json_object_new_int(analyzer_info->SW_CNT[j]));
			}
			for(j = 0; j < analyzer_info->cnt_random; j++)
			{		
				memset(IDStr, 0, 10);
				sprintf(IDStr, "%d", j);
				json_object_object_add(rrc, IDStr, json_object_new_int(analyzer_info->RR_CNT[j]));
				json_object_object_add(rwc, IDStr, json_object_new_int(analyzer_info->RW_CNT[j]));
			}
			
			json_object_object_add(janalyzer, "SR", src);
			json_object_object_add(janalyzer, "SW", swc);
			json_object_object_add(janalyzer, "RR", rrc);
			json_object_object_add(janalyzer, "RW", rwc);
		}
		else
		{
			json_object_object_add(janalyzer, "Enable", json_object_new_int(0));
		}

		json_object_object_add(jobj, "iAnalyzer", janalyzer);
		json_object_array_add(ret, jobj);
		
		free(IDStr);

		attrib_release(attrib_info, NULL);

		node = (STORAGE_INFO*)node->next_node;
	}
	return ret;
}

struct json_object* GetGPUInfo(GPU_INFO* ptr){
	struct json_object* ret = json_object_new_object();
#ifdef __aarch64__
	json_object_object_add(ret, "Name", json_object_new_string(ptr->Name));
	json_object_object_add(ret, "Arch", json_object_new_string(ptr->Arch));
	json_object_object_add(ret, "DriverVer", json_object_new_string(ptr->DriverVer));
	json_object_object_add(ret, "ComputeCap", json_object_new_string(ptr->ComputeCap));
	json_object_object_add(ret, "CoreNum", json_object_new_string(ptr->CoreNum));
	json_object_object_add(ret, "MemType", json_object_new_string(ptr->MemType));
	json_object_object_add(ret, "MemBusWidth", json_object_new_string(ptr->MemBusWidth));
	json_object_object_add(ret, "MemSize", json_object_new_string(ptr->MemSize));
	json_object_object_add(ret, "MemBandWidth", json_object_new_string(ptr->MemBandWidth));
	json_object_object_add(ret, "Clock", json_object_new_string(ptr->Clock));
	json_object_object_add(ret, "CoreNum", json_object_new_string(ptr->CoreNum));
	
#endif
	return ret;
}

struct json_object* GetGPUDynamicInfo(GPU_DYNAMIC_INFO* ptr){
	struct json_object* ret = json_object_new_object();

	json_object_object_add(ret, "CoreClock", json_object_new_int(ptr->CoreClock));
	json_object_object_add(ret, "FanTemp", json_object_new_int(ptr->FanTemp));
	json_object_object_add(ret, "Load", json_object_new_int(ptr->Load));
	json_object_object_add(ret, "MemUsed", json_object_new_int(ptr->MemUsed));
	json_object_object_add(ret, "Temp", json_object_new_int(ptr->Temp));
	return ret;
}
float Kelvins2Celsius(int k) // (0.1K)
{
	return (k / 10.0 - 273.15);
}

void AddEApiVoltageValue(struct json_object* src, const char* key, struct json_object* ret)
{
	struct json_object* tmp = get_json_object(src, key);
	if (tmp != NULL)
		json_object_object_add(ret, key, json_object_new_double(json_object_get_int(tmp) / 1000.0));
}

void AddEApiTempValue(struct json_object* src, const char* key, struct json_object* ret)
{
	struct json_object* tmp = get_json_object(src, key);
	if (tmp != NULL)
		json_object_object_add(ret, key, json_object_new_double(Kelvins2Celsius(json_object_get_int(tmp))));
}

struct json_object* GetEApiDynamicInfo(char* json_str)
{	
	struct json_object *root, *hwmon, *voltage, *temp, *fan;
	struct json_object* ret = json_object_new_object();
	struct json_object* sub_hwmon = json_object_new_object();

	if (json_str == NULL) {
		json_object_put(sub_hwmon);
		return ret;
	}
	
	root = json_tokener_parse(json_str);
	if (root == NULL) {
		json_object_put(sub_hwmon);
		return ret;
	}
 
	hwmon = get_json_object(root, "HWMON");
	if (hwmon == NULL) {
		json_object_put(sub_hwmon);
		return ret;
	}

	voltage = get_json_object(hwmon, "VOLTAGE");
	if (voltage != NULL) {
		struct json_object* sub_voltage = json_object_new_object();

		AddEApiVoltageValue(voltage, "VCORE", sub_voltage);
		AddEApiVoltageValue(voltage, "3V3", sub_voltage);
		AddEApiVoltageValue(voltage, "VBAT", sub_voltage);
		AddEApiVoltageValue(voltage, "5V", sub_voltage);
		AddEApiVoltageValue(voltage, "5VSB", sub_voltage);
		AddEApiVoltageValue(voltage, "+12V", sub_voltage);
		AddEApiVoltageValue(voltage, "3VSB", sub_voltage);
		AddEApiVoltageValue(voltage, "VCORE2", sub_voltage);
		AddEApiVoltageValue(voltage, "VGFX", sub_voltage);
		AddEApiVoltageValue(voltage, "AVSB", sub_voltage);
		AddEApiVoltageValue(voltage, "1V05", sub_voltage);
		AddEApiVoltageValue(voltage, "VCCSA", sub_voltage);
		AddEApiVoltageValue(voltage, "VDDQ", sub_voltage);
		AddEApiVoltageValue(voltage, "VCCGT", sub_voltage);
		AddEApiVoltageValue(voltage, "VCCIO", sub_voltage);
		AddEApiVoltageValue(voltage, "VNN", sub_voltage);
		AddEApiVoltageValue(voltage, "1V0", sub_voltage);
		AddEApiVoltageValue(voltage, "VDDNB", sub_voltage);
		AddEApiVoltageValue(voltage, "1V8", sub_voltage);
		AddEApiVoltageValue(voltage, "0V95", sub_voltage);
		AddEApiVoltageValue(voltage, "1V15", sub_voltage);

		json_object_object_add(sub_hwmon, "VOLTAGE", sub_voltage);
	}

	temp= get_json_object(hwmon, "TEMP");
	if (temp != NULL) {
		struct json_object* sub_temp = json_object_new_object();
		
		AddEApiTempValue(temp, "CPUTEMP", sub_temp);
		AddEApiTempValue(temp, "CHIPTEMP", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP2", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP3", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP4", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP5", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP6", sub_temp);
		AddEApiTempValue(temp, "SYSTEMP7", sub_temp);
		AddEApiTempValue(temp, "CPU2TEMP", sub_temp);
	
		json_object_object_add(sub_hwmon, "TEMP", sub_temp);
	}

	fan = get_json_object(hwmon, "FAN");
	if (fan != NULL)
		json_object_object_add(sub_hwmon, "FAN", fan);

	json_object_object_add(ret, "HWMON", sub_hwmon);
	return ret;
}

// int CheckInno(STORAGE_INFO* ptr)
// {
// 	int NumberOfInnoStorage = 0;
// 	STORAGE_INFO* node = ptr;

// 	while(node != NULL) {
// #ifdef WIN32
// 		set_analyzer(node->Index, 1);
// 		if (is_inno(node->Index))
// #else
// 		set_analyzer(node->Location, 1);
// 		if (is_inno(node->Location))
// #endif
// 		{
// 			NumberOfInnoStorage++;
// 		}
// 		node = (STORAGE_INFO*)node->next_node;
// 	}
// 	return NumberOfInnoStorage;
// }

int CheckStorChange()
{
	if(StorNum!=StorList.size())
	{
		StorNum = StorList.size();
		return 1;
	}
	else
	{
		return 0;
	}
	
}

void GetLocalStor(STORAGE_INFO* ptr)
{
	STORAGE_INFO* node = ptr;
	STORAGE_PAR_INFO* subnode = NULL;
	StorList.clear();
	while(node != NULL)
	{
		DISK_INFO disk_info;
		IDENTIFY_INFO *id_info;

#ifdef WIN32
		set_analyzer(node->Index, 1);
		get_dev_info(node->Index, &disk_info);
		id_info = &disk_info.id;
#else
		set_analyzer(node->Location, 1);
		get_dev_info(node->Location, &disk_info);
		id_info = &disk_info.id;
#endif
		char* SNStr = (char*)malloc(sizeof(char) * 50);
		memset(SNStr,0,50);
		strncpy(SNStr, id_info->SerialNum, SN_LEN);
		SNStr = lib_SysInfo_TrimWhiteSpace(SNStr);

		if(strncmp(disk_info.id.ModelName,"ASM",3) !=0 && strncmp(disk_info.id.ModelName,"JMI",3)!=0)
		{
			if(string(SNStr)!="")
			{
				StorList.insert(string(SNStr));
				InsertStorageData(SNStr, (float)(node->Capacity / 1024.0 / 1024.0), (float)disk_info.dev.Health, disk_info.dev.PECycle);
			}
		}
		
		free(SNStr);
		node = (STORAGE_INFO*)node->next_node;
	}
#ifdef WIN32
	if(CheckEXSSInno())	GetEXSS_StorInfo();
	// if(CheckEXPSInno())	GetEXPS_StorInfo();
#endif
}
