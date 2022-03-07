#ifndef __LIBSMART_H__
#define __LIBSMART_H__

#include <stdbool.h>
#include <stdint.h>

/*-----------------------------------------------------------------------------------------------------------*/
#define DIM(var, type)  (sizeof(var)) / sizeof(type)

/*-----------------------------------------------------------------------------------------------------------*/
#define MN_LEN      40
#define SN_LEN      20
#define FW_LEN      8
#define RAW_LEN     12
#define WWN_LEN     16
#define MAX_RAID    8

/*-----------------------------------------------------------------------------------------------------------*/
/* customized SN, IDTable start from byte 258 */
#define CUST_SN_LEN 12

/*-----------------------------------------------------------------------------------------------------------*/
enum 
{
    SE_DESTROY                      = 0x20,
    SE_QERASE                       = 0x21,
    SE_AFFSI_5020,                 
    SE_DOD_5220_22_M,
    SE_USA_Navy_NAVSO_P_5239_26,
    SE_NSA_Manual_130_2,
    SE_USA_Army_380_19,
    SE_NISPOMSUP_Chap_8_Sect_8_501,
    SE_NSA_Manual_9_12,
    SE_IRIG_106,
    SE_NONE,
};

/*-----------------------------------------------------------------------------------------------------------*/
typedef struct
{
    int yy;
    int mm;
    int dd;

    float health;
    char  sn[SN_LEN+1]; 

} LIFESPAN_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
typedef struct
{
    int ver;   

    int cnt_sequential;             /* sequential read & write item count */
    int cnt_random;                 /* random read & write item count     */

    unsigned char TR_PCT[2];        /* total read percentage              */
    unsigned char TW_PCT[2];        /* total write percentage             */
    unsigned char SR_PCT[7];        /* sequential read percentage         */
    unsigned char SW_PCT[7];        /* sequential write Percentage        */
    unsigned char RR_PCT[7];        /* random read percentage             */
    unsigned char RW_PCT[7];        /* random write percentage            */

    unsigned long TR_CNT[2];        /* total read percentage              */
    unsigned long TW_CNT[2];        /* total write percentage             */
    unsigned long SR_CNT[7];        /* sequential read percentage         */
    unsigned long SW_CNT[7];        /* sequential write Percentage        */
    unsigned long RR_CNT[7];        /* random read Percentage             */
    unsigned long RW_CNT[7];        /* random write Percentage            */      

    unsigned char buf[512];

} ANALYZER_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
typedef struct
{
    struct
    {
        int s;
        int e;
    } offset;

    unsigned char id;
    char          *desc;
    long long     val;
    unsigned char raw[RAW_LEN];
} ATTRIB_INFO;

typedef struct
{
    unsigned char buf[512];

    int           attrib_cnt;
    ATTRIB_INFO   *attrib;

    /* NVMe SMART */
    int           health_cnt;
    ATTRIB_INFO   *health;
            
} SMART_INFO;

typedef struct
{
    char TransferMode[32];
    char Interface[32];
    char Standard[32];

    bool LBA48;
    bool HPA;
    bool NCQ;
    bool TRIM;
    bool Security;
    bool TSensor;

    bool InnoRobust;
    bool SErase;
    bool QErase;
    bool Destroy;
    bool Crypto;

    /* -1 : w/o support */
    /* 0  : disable     */
    /* 1  : enable      */
    int WProtect;
    int Analyzer;

} FEATURE_INFO;

typedef struct
{
    unsigned char buf[512];

    char ModelName[MN_LEN + 1];
    char SerialNum[SN_LEN + 1];
    char FWVer[FW_LEN + 1];
    char CustSN[SN_LEN + 1];
    char EUI64_WWN[WWN_LEN + 1];

    FEATURE_INFO feature;

} IDENTIFY_INFO;

typedef struct
{
    unsigned char id[4096];
    unsigned char id_ns[4096];
    unsigned char smart[4096];
    unsigned char health[4096];    
} NVME_DATA;

typedef struct
{
    int           BusType;
    bool          OSR;
    int           Temperature;
    float         Health;
    float         Capacity;
    long long     LBAs; 
    long long     NativeLBAs; 
    int           PECycle;
    int           AvgErase;
    int           PowerOnHours;
    int           PowerOnCycles;
    int           InitSpare;
    int           LaterBad;
    unsigned long TotalLBAWritten; 
} DEV_INFO;

typedef struct
{
    IDENTIFY_INFO id;
    SMART_INFO    smart;   
    ANALYZER_INFO analyzer; 
    NVME_DATA     nvme_data;
    DEV_INFO      dev;

    /*------------------------------*/
    bool          isRAID;
    int           raid_cnt;
    bool          raid_status[MAX_RAID];
    IDENTIFY_INFO raid_id[MAX_RAID];
    SMART_INFO    raid_smart[MAX_RAID];   
    ANALYZER_INFO raid_analyzer[MAX_RAID]; 
    DEV_INFO      raid_dev[MAX_RAID];
} DISK_INFO;

typedef struct
{
    LIFESPAN_INFO   life_info;

    float           Health;
    int             lifespan;
    int             PECycle;
    bool            isInno;
} BASIC_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
/* InnoAGE WiFi */
enum
{
    SET_SSID    = 1,
    SCAN_WiFi,
    READ_WiFi,
    DEL_WiFi,
};

typedef struct
{
    char SSID[32];
    char password[32];
} SSID_SET;

typedef struct
{
    char SSID[33];
    char db[5];
} WIFI_INFO;

typedef struct
{
    int           cmd;
    int           idx;
    unsigned char data[512];
    SSID_SET      set;
    WIFI_INFO     Wifi[32];
} INNOAGE_WIFI_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
/* InnoOSR */
enum
{
    OSR_COPY                    = 0,
    OSR_READ_PTN,
    OSR_BACKUP,
    OSR_RECOVERY,
    OSR_CHECK_STATUS,
    OSR_CLEAN_STATUS,
    OSR_SET_COPY_LBA,
    OSR_SET_RSV_SECTOR,
    OSR_WRITE_PROTECT_ENABLE,    
    OSR_WRITE_PROTECT_DISABLE    
};

enum
{
    ERR_4K_ALIGN            = -99,
    ERR_NOT_ENOUGH_SPACE,
    ERR_ENABLED,
};

#pragma pack(push, 1)
typedef struct
{
    /* Byte 0 */
    bool isSetCopyLBA : 1;

    /* Byte 1 */
    struct
    {
        bool isBackupEnd            : 1;
        bool isBackupWriteFail      : 1;
        bool isBackupReadECCFail    : 1;
        bool isBackupStart          : 1;

        bool isRecoveryEnd          : 1;
        bool isRecoveryWriteFail    : 1;
        bool isRecoveryReadECCFail  : 1;
        bool isRecoveryStart        : 1;
    };   

    /* Byte 2 ~ 3 */
    struct
    {
        unsigned short hidden_size : 15;
        unsigned short isSetHidden : 1;
    }; 

    uint32_t copy_lba_start;       /* Byte 4  ~ 7  */
    uint32_t copy_lba_end;         /* Byte 8  ~ 11 */
    uint32_t target_lba_start;     /* Byte 12 ~ 15 */
    uint32_t write_protect_start;  /* Byte 16 ~ 19 */
    uint32_t write_protect_end;    /* Byte 20 ~ 23 */
    uint32_t reserved_size_sector; /* Byte 24 ~ 27 */
    unsigned char back_recv_pct;        /* Byte 28      */
} OSR_STATUS;
#pragma pack(pop)

typedef struct
{
    long long head;
    long long tail;
} PTN_INFO;

typedef struct
{
    int           n;
    PTN_INFO      ptn[16];       /* read partition         */
    long long     rev_cp_sector; /* reserved & copy sector */
    unsigned char status[512];   /* status buf             */
} OSR_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
typedef struct 
{
    long          blk_unit;
    long          erase_cnt[8192];
    unsigned char raw[8192 * 4];   /* erase cnt raw */

    unsigned long TableEraseCnt;
    unsigned long UserEraseCnt;
    unsigned long AvgTableEraseCnt;
    unsigned long AvgUserEraseCnt;
    unsigned long UserBlockCnt;
} ERASE_CNT_INFO;

/*-----------------------------------------------------------------------------------------------------------*/
typedef struct
{
    bool          bIsRead;
    unsigned char data[50];
} USER_DATA;

/*-----------------------------------------------------------------------------------------------------------*/
#ifdef _WIN32
    extern unsigned long get_dev_info(int disk, DISK_INFO *info);
    extern int get_lifespan(int disk, LIFESPAN_INFO *info);
    extern bool get_vmware_smart(int, DISK_INFO *info, int mode);
    extern void set_analyzer(int disk, int set);
    extern void set_write_protect(int disk, int set);
    extern unsigned long security_erase(int disk, char serase);
    extern unsigned long crypto_erase(int disk);
    extern int get_serase_status(int disk);
    extern int get_serase_status_nvme(int disk);
    extern void get_lba_info(int disk, int lba, unsigned char *buf, int size);    
    extern int get_native_lba(int disk, long long *max);
    extern int set_native_lba(int disk, long long lba);
    extern bool is_inno(int disk);

    /* InnoAGE WIFI */
    extern unsigned long InnoAGE_WiFi(int disk, INNOAGE_WIFI_INFO *WiFi);

    /* MK */
    extern unsigned long get_erase_cnt(int disk, ERASE_CNT_INFO *info);

    /* MK S19910W only */
    extern unsigned long set_cust_sn(int disk, char *sn);
    extern unsigned long rw_user_data(int disk, USER_DATA *cust);
    extern unsigned long ata_se_qe(int disk);

    /* InnoOSR */
    int execOSR(int disk, int cmd, OSR_INFO *info);

    extern void umount_disk(int disk);
    extern void mount_disk(int disk, char letter);
#else
    extern int get_dev_info(char *disk, DISK_INFO *info);
    extern int get_lifespan(char *disk, LIFESPAN_INFO *info);
    extern bool get_vmware_smart(char *disk, DISK_INFO *info, int mode);
    extern void set_analyzer(char *disk, int set);
    extern void set_write_protect(char *disk, int set);
    extern int security_erase(char *disk, char serase);
    extern int crypto_erase(char *disk);
    extern int get_serase_status(char *disk);
    extern int get_serase_status_nvme(char *disk);   
    extern int get_native_lba(char *disk, long long *max);
    extern int set_native_lba(char *disk, long long lba);
    extern bool is_inno(char *disk);

    /* SSID */
    extern int InnoAGE_WiFi(char *disk, INNOAGE_WIFI_INFO *WiFi);

    /* MK */
    extern int get_erase_cnt(char *disk, ERASE_CNT_INFO *info);

    /* MK S19910W only */
    extern int set_cust_sn(char *disk, char *sn);
    extern int rw_user_data(char *disk, USER_DATA *user);
    extern int ata_se_qe(char *disk);

    /* InnoOSR */
    int execOSR(char *disk, int cmd, OSR_INFO *info);

    #ifdef OS_SOLARIS
        extern void set_se_timeout(int sec);
    #endif
#endif

extern char* get_build_date(void); 
extern char* get_lib_ver(void);
extern void  attrib_release(ATTRIB_INFO *attrib, ATTRIB_INFO *health);
extern int   get_sata_smart(unsigned char *id, SMART_INFO *smart, BASIC_INFO *basic);

#endif

