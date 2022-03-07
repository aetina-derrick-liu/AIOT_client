#ifdef WIN32
#include <windows.h>
#include <iostream>
#include <unordered_set>
#include "DBInterface.hpp"

extern "C"
{
	#include "lib_ExSS.h"
    #include "lib_ExPS.h"
	#include "libsmart.h"
}

using namespace std;
extern unordered_set <string> StorList;
typedef char *(*GET_LIB_VER) (void);
typedef int (*GET_SATA_SMART) (unsigned char *id, SMART_INFO *smart, BASIC_INFO *basic);
/********************************************************/
//EXSS
EXSS_RAID_CREATE        ExSSRaidCreate = NULL;
EXSS_GET_RAID_INFO      ExSSGetRaidInfo = NULL;
EXSS_GET_SMART_ID_TABLE ExSSGetSmartIDTable = NULL;
EXSS_MONITOR            ExSSMonitor = NULL;
EXSS_GET_REBUILD_PER    ExSSGetRebuildPer = NULL;
//EXPS
EXPS_LOAD_ASMIO_DRIVER  ExPSLoadAsmIODriver = NULL;
EXPS_RAID_CREATE        ExPSRaidCreate = NULL;
EXPS_GET_RAID_INFO      ExPSGetRaidInfo = NULL;
EXPS_GET_SMART_ID_TABLE ExPSGetSmartIDTable = NULL;
EXPS_MONITOR            ExPSMonitor = NULL;
/******************************************************/

GET_LIB_VER    get_ismart_ver = NULL;
GET_SATA_SMART get_sata_smart_raid = NULL;
GET_SATA_SMART get_sata_smart_ = NULL;

HMODULE hDLL_ExSS = NULL;
HMODULE hDLL_ExPS = NULL;
HMODULE hDLL_iSMART = NULL;

int load_EXSS_dll (void);
int load_EXPS_dll (void);
/*********************************************************************************/
//EXSS
int CheckEXSSInno()
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    SMART_INFO         smart_info;
    EXSS_RAID_INFO     exss_raid_info;
    EXSS_SMART_ID      exss_smart_id;  
    EXSS_MONITOR_INFO  exss_monitor_info;
    BASIC_INFO         basic;

    if(load_EXSS_dll())
    {
        return -1;
    }
    /* ExSSGetRaidInfo */
    memset(&exss_raid_info, 0, sizeof(EXSS_RAID_INFO));
    avai_ctrl = ExSSGetRaidInfo(&exss_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExPS device!\n");
        return -1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exss_smart_id, 0, sizeof(EXSS_SMART_ID));
        ExSSGetSmartIDTable(i, &exss_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_SUPPORT; j++)
        {
            memset(&basic,0,sizeof(BASIC_INFO));
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memcpy(&smart_info.buf[0], &exss_smart_id.SMARTTable[j][0], 512);
            get_sata_smart(&exss_smart_id.IDTable[j][0], &smart_info, &basic);
            if(basic.isInno)           return 1;
        }
    }
    return 0;
}
int GetEXSS_StorInfo()
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    SMART_INFO         smart_info;
    EXSS_RAID_INFO     exss_raid_info;
    EXSS_SMART_ID      exss_smart_id;
    EXSS_MONITOR_INFO  exss_monitor_info;
    BASIC_INFO         basic;

    if(load_EXSS_dll())
    {
        return 1;
    }

    /* ExSSGetRaidInfo */
    memset(&exss_raid_info, 0, sizeof(EXSS_RAID_INFO));
    avai_ctrl = ExSSGetRaidInfo(&exss_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExSS device!\n");
        return 1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exss_smart_id, 0, sizeof(EXSS_SMART_ID));
        ExSSGetSmartIDTable(i, &exss_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_RAID; j++)
        {
            memset(&basic,0,sizeof(BASIC_INFO));
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memcpy(&smart_info.buf[0], &exss_smart_id.SMARTTable[j][0], 512);
            get_sata_smart(&exss_smart_id.IDTable[j][0], &smart_info, &basic);
            if(basic.PECycle!=-1)
            {
                StorList.insert(string(exss_raid_info.ExSSDiskSN[i][j]));
		        InsertStorageData(exss_raid_info.ExSSDiskSN[i][j],atof(exss_raid_info.ExSSDiskcapacity[i][j]),basic.Health,basic.PECycle);
            }
        }
    }
}

int GetEXSS_StorLife(int lastRawId)
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    int                avgerasecnt;
    int                temp;
    SMART_INFO         smart_info;
    EXSS_RAID_INFO     exss_raid_info;
    EXSS_SMART_ID      exss_smart_id;
    EXSS_MONITOR_INFO  exss_monitor_info;
    BASIC_INFO         basic;
    LIFESPAN_INFO      life; 
    

    if(load_EXSS_dll())
    {
        return 1;
    }

    /* ExSSGetRaidInfo */
    memset(&exss_raid_info, 0, sizeof(EXSS_RAID_INFO));
    avai_ctrl = ExSSGetRaidInfo(&exss_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExSS device!\n");
        return 1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exss_smart_id, 0, sizeof(EXSS_SMART_ID));
        ExSSGetSmartIDTable(i, &exss_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_RAID; j++)
        {
            char* time=NULL;
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memset(&basic,0,sizeof(BASIC_INFO));
            memcpy(&smart_info.buf[0], &exss_smart_id.SMARTTable[j][0], 512);

            char* SNStr=(char*)malloc(sizeof(char)*20);
            memset(SNStr,0,20);
            strncpy(SNStr,exss_raid_info.ExSSDiskSN[i][j],strlen(exss_raid_info.ExSSDiskSN[i][j])+1);
            time=GetStorageDataLastModifyDate(SNStr);
			int YY,MM,DD,hh,mm,ss;
			int lifespan;
			sscanf(time,"%d-%d-%d %d:%d:%d",&YY,&MM,&DD,&hh,&mm,&ss);
			life.yy=YY;
			life.mm=MM;
			life.dd=DD;
			life.health=GetStorageInitHealth(SNStr);
			strcpy(life.sn,SNStr);
            basic.life_info=life;
            get_sata_smart(&exss_smart_id.IDTable[j][0], &smart_info, &basic);

            for(k=0; k<smart_info.attrib_cnt; k++)
            {
#ifdef DEBUG
                printf("\t[%02X]  %-42s  [%5I64d]\n",
                smart_info.attrib[k].id,
                smart_info.attrib[k].desc,
                smart_info.attrib[k].val);
#endif
                if(smart_info.attrib[k].id==0xA7)
                {
                    avgerasecnt=smart_info.attrib[k].val;
                }
                if(smart_info.attrib[k].id==0xC2)
                {
                    temp=smart_info.attrib[k].val;
                }
			}
            if(basic.PECycle!=-1)
            {
                StorList.insert(string(exss_raid_info.ExSSDiskSN[i][j]));
                InsertStorageRawData(exss_raid_info.ExSSDiskSN[i][j], lastRawId, basic.Health, temp, avgerasecnt);
                CheckStorageThreshold(SNStr, temp, avgerasecnt, lifespan);
            }
            free(SNStr);
        }
    }
}

int load_EXSS_dll (void)
{
    const char *lib_exss = "lib_ExSS.dll";
    const char *lib_ismart = "libsmart.dll";

    hDLL_ExSS = LoadLibrary(lib_exss);
    if(NULL == hDLL_ExSS)
    {
        printf("Open %s failed !\n", lib_exss);
        return 1;
    }
    else
    {
        ExSSRaidCreate      = (EXSS_RAID_CREATE)        GetProcAddress(hDLL_ExSS, "ExSSRaidCreate");
        ExSSGetRaidInfo     = (EXSS_GET_RAID_INFO)      GetProcAddress(hDLL_ExSS, "ExSSGetRaidInfo");
        ExSSGetSmartIDTable = (EXSS_GET_SMART_ID_TABLE) GetProcAddress(hDLL_ExSS, "ExSSGetSmartIDTable");
        ExSSMonitor         = (EXSS_MONITOR)            GetProcAddress(hDLL_ExSS, "ExSSMonitor");
        ExSSGetRebuildPer   = (EXSS_GET_REBUILD_PER)    GetProcAddress(hDLL_ExSS, "ExSSGetRebuildPer");

        if(!ExSSRaidCreate || !ExSSGetRaidInfo || !ExSSMonitor || !ExSSGetRebuildPer || !ExSSGetSmartIDTable)
        {
            printf("Get function from %s failed !\n", lib_exss);
            return 1;
        }
    }

    hDLL_iSMART = LoadLibrary(lib_ismart);
    if(NULL == hDLL_iSMART)
    {
        printf("Open %s failed !\n", lib_ismart);
        return 1;
    }
    else
    {
        get_sata_smart_raid = (GET_SATA_SMART) GetProcAddress(hDLL_iSMART, "get_sata_smart");
        get_ismart_ver = (GET_LIB_VER)    GetProcAddress(hDLL_iSMART, "get_lib_ver");

        if(!get_sata_smart_raid || !get_ismart_ver)
        {
            printf("Get function from %s failed !\n", lib_ismart);
            return 1;
        }
    }

    return 0;
}
/*************************************************************************************************************/
//EXPS
int load_EXPS_dll (void);

int CheckEXPSInno()
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    SMART_INFO         smart_info;
    EXPS_RAID_INFO     exps_raid_info;
    EXPS_SMART_ID      exps_smart_id;  
    EXPS_MONITOR_INFO  exps_monitor_info;
    BASIC_INFO         basic;

    if(load_EXPS_dll())
    {
        return -1;
    }
    if(ExPSLoadAsmIODriver())
    {
        printf("ExPS load ASM IO driver failed!\n");
        return -1;
    }

    /* ExPSGetRaidInfo */
    memset(&exps_raid_info, 0, sizeof(EXPS_RAID_INFO));
    avai_ctrl = ExPSGetRaidInfo(&exps_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExPS device!\n");
        return -1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exps_smart_id, 0, sizeof(EXPS_SMART_ID));
        ExPSGetSmartIDTable(i, &exps_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_SUPPORT; j++)
        {
            memset(&basic,0,sizeof(BASIC_INFO));
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memcpy(&smart_info.buf[0], &exps_smart_id.SMARTTable[j][0], 512);
            get_sata_smart(&exps_smart_id.IDTable[j][0], &smart_info, &basic);
            if(basic.isInno)           return 1;
        }
    }
    return 0;
}
int GetEXPS_StorInfo()
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    SMART_INFO         smart_info;
    EXPS_RAID_INFO     exps_raid_info;
    EXPS_SMART_ID      exps_smart_id;
    EXPS_MONITOR_INFO  exps_monitor_info;
    BASIC_INFO         basic;

    if(load_EXPS_dll())
    {
        return -1;
    }
    if(ExPSLoadAsmIODriver())
    {
        printf("ExPS load ASM IO driver failed!\n");
        return -1;
    }

    /* ExPSGetRaidInfo */
    memset(&exps_raid_info, 0, sizeof(EXPS_RAID_INFO));
    avai_ctrl = ExPSGetRaidInfo(&exps_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExPS device!\n");
        return 1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exps_smart_id, 0, sizeof(EXPS_SMART_ID));
        ExPSGetSmartIDTable(i, &exps_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_SUPPORT; j++)
        {
            memset(&basic,0,sizeof(BASIC_INFO));
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memcpy(&smart_info.buf[0], &exps_smart_id.SMARTTable[j][0], 512);
            get_sata_smart(&exps_smart_id.IDTable[j][0], &smart_info, &basic);
            if(basic.PECycle!=-1)
            {
                StorList.insert(string(exps_raid_info.ExPSDiskSN[i][j]));
		        InsertStorageData(exps_raid_info.ExPSDiskSN[i][j],atof(exps_raid_info.ExPSDiskcapacity[i][j]),basic.Health,basic.PECycle);
            }
        }
    }
}

int GetEXPS_StorLife(int lastRawId)
{
	int                i;
    int                j;
    int                k;
    int                avai_ctrl;
    int                avgerasecnt;
    int                temp;
    SMART_INFO         smart_info;
    EXPS_RAID_INFO     exps_raid_info;
    EXPS_SMART_ID      exps_smart_id;
    EXPS_MONITOR_INFO  exps_monitor_info;
    BASIC_INFO         basic;
    LIFESPAN_INFO      life; 
 
    if(load_EXPS_dll())
    {
        return 1;
    }
    if(ExPSLoadAsmIODriver() == 1)
    {
        printf("ExPS load ASM IO driver failed!\n");
        return 1;
    }

    /* ExPSGetRaidInfo */
    memset(&exps_raid_info, 0, sizeof(EXPS_RAID_INFO));
    avai_ctrl = ExPSGetRaidInfo(&exps_raid_info);

    if(avai_ctrl <= 0)
    {
        printf("No ExPS device!\n");
        return 1;
    }

    for(i=0; i<avai_ctrl; i++)
    {
        /* ExSSGetSmartIDTable */
        memset(&exps_smart_id, 0, sizeof(EXPS_SMART_ID));
        ExPSGetSmartIDTable(i, &exps_smart_id);

        /* disk */
        for(j=0; j<MAX_DISK_IN_SUPPORT; j++)
        {
            char* time=NULL;
            memset(&smart_info, 0, sizeof(SMART_INFO));
            memset(&basic,0,sizeof(BASIC_INFO));
            memcpy(&smart_info.buf[0], &exps_smart_id.SMARTTable[j][0], 512);

            char* SNStr=(char*)malloc(sizeof(char)*20);
            memset(SNStr,0,20);
            strncpy(SNStr,exps_raid_info.ExPSDiskSN[i][j],strlen(exps_raid_info.ExPSDiskSN[i][j])+1);
            time=GetStorageDataLastModifyDate(SNStr);
			int YY,MM,DD,hh,mm,ss;
			int lifespan;
			sscanf(time,"%d-%d-%d %d:%d:%d",&YY,&MM,&DD,&hh,&mm,&ss);
			life.yy=YY;
			life.mm=MM;
			life.dd=DD;
			life.health=GetStorageInitHealth(SNStr);
			strcpy(life.sn,SNStr);
            basic.life_info=life;
            get_sata_smart(&exps_smart_id.IDTable[j][0], &smart_info, &basic);

            for(k=0; k<smart_info.attrib_cnt; k++)
            {
#ifdef DEBUG
                printf("\t[%02X]  %-42s  [%5I64d]\n",
                smart_info.attrib[k].id,
                smart_info.attrib[k].desc,
                smart_info.attrib[k].val);
#endif
                if(smart_info.attrib[k].id==0xA7)
                {
                    avgerasecnt=smart_info.attrib[k].val;
                }
                if(smart_info.attrib[k].id==0xC2)
                {
                    temp=smart_info.attrib[k].val;
                }
			}
            if(basic.PECycle!=-1)
            {
                InsertStorageRawData(exps_raid_info.ExPSDiskSN[i][j], lastRawId, basic.Health, temp, avgerasecnt);
                CheckStorageThreshold(SNStr, temp, avgerasecnt, lifespan);
            }
            free(SNStr);
        }
    }
}

int load_EXPS_dll (void)
{
    const char *lib_exps = "lib_ExPS.dll";
    const char *lib_ismart = "libsmart.dll";

    hDLL_ExPS = LoadLibrary(lib_exps);
    if(NULL == hDLL_ExPS)
    {
        printf("Open %s failed !\n", lib_exps);
        return 1;
    }
    else
    {
        ExPSLoadAsmIODriver = (EXPS_LOAD_ASMIO_DRIVER)  GetProcAddress(hDLL_ExPS, "ExPSLoadAsmIODriver");
        ExPSRaidCreate      = (EXPS_RAID_CREATE)        GetProcAddress(hDLL_ExPS, "ExPSRaidCreate");
        ExPSGetRaidInfo     = (EXPS_GET_RAID_INFO)      GetProcAddress(hDLL_ExPS, "ExPSGetRaidInfo");
        ExPSGetSmartIDTable = (EXPS_GET_SMART_ID_TABLE) GetProcAddress(hDLL_ExPS, "ExPSGetSmartIDTable");
        ExPSMonitor         = (EXPS_MONITOR)            GetProcAddress(hDLL_ExPS, "ExPSMonitor");

        if(!ExPSLoadAsmIODriver || !ExPSRaidCreate || !ExPSGetRaidInfo || !ExPSMonitor || !ExPSGetSmartIDTable)
        {
            printf("Get function from %s failed !\n", lib_exps);
            return 1;
        }
    }

    hDLL_iSMART = LoadLibrary(lib_ismart);
    if(NULL == hDLL_iSMART)
    {
        printf("Open %s failed !\n", lib_ismart);
        return 1;
    }
    else
    {
        get_sata_smart_ = (GET_SATA_SMART) GetProcAddress(hDLL_iSMART, "get_sata_smart");
        get_ismart_ver = (GET_LIB_VER)    GetProcAddress(hDLL_iSMART, "get_lib_ver");

        if(!get_sata_smart_ || !get_ismart_ver)
        {
            printf("Get function from %s failed !\n", lib_ismart);
            return 1;
        }
    }

    return 0;
}


#endif
