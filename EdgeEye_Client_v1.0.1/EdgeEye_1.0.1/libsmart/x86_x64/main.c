#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "libsmart.h"

#define SHOW_ANALYZER

#if defined(OS_WIN)
    extern void OSRTest(int disk, DISK_INFO info);
#else
    extern void OSRTest(char *disk, DISK_INFO info);
#endif

/*-----------------------------------------------------------------------------------------------------------*/
enum
{
    MODE_NONE   = -1,
    MODE_VM,
    MODE_VM_SRR
};

/*-----------------------------------------------------------------------------------------------------------*/
static char *analyzer_sequential_v0[] = { "8M", "4M", "1M", "128K", "64K", "32K" };
static char *analyzer_random_v0[]     = { "64K", "32K", "16K", "8K", "4K" };
static char *analyzer_v1[]            = { "128.x K", "64.x K", "32.x K", "16.x K", "8.x K", "4.x K", "0.x K" };

/*-----------------------------------------------------------------------------------------------------------*/
static const char *erase_str[] =
{
    "Destroy",
    "Quick Erase",
    "USA-AF AFSSI 5020",
    "DoD 5220.22-M",
    "USA Navy NAVSO P-5239-26",
    "NSA Manual 130-2",
    "USA-Army 380-19",
    "NISPOMSUP Chap 8, Sect. 8-501",
    "NSA Manual 9-12",
    "IRIG 106",
};  

/*-----------------------------------------------------------------------------------------------------------*/
static const struct option long_opt[] =
{
    { "Device",          1, NULL, 'd' },   
    { "iAnalyzer",       1, NULL, 'a' },   
    { "Write Protect",   1, NULL, 'w' },   
    { "Security Erase",  1, NULL, 'e' },   
    { "crypto erase",    1, NULL, 'c' },   
    { "VMware",          1, NULL, 'v' },   
    { "SE timeout",      1, NULL, 't' },   
    { "Get Erase Cnt",   1, NULL, 'g' },   
    { "Set LBA via HPA", 1, NULL, 's' },   
    { "OSR Test",        1, NULL, 'r' },   
    { "Help",            1, NULL, 'h' },   
    { NULL,              0, NULL, '0' },
};

/*-----------------------------------------------------------------------------------------------------------*/
static void show_usage(char *name, bool show_title_only)
{
    int i = 0;

    printf("************************************************************************\n");
    printf("* Innodisk iSMART %s                                   %s *\n", get_lib_ver(), get_build_date());
    printf("************************************************************************\n");

    if(show_title_only)
        return;

    #if defined(OS_WIN)
        printf("Syntax: %s [-option ...] \n", name);
        printf("Sample: %s -d 0          \n", name);    
        printf("Option:\n");
        printf("  -d : device \n");
        printf("  -a : iAnalyzer (0:Disable 1:Enable 2:Clean) \n");
        printf("  -w : write protect (0:Disable 1:Enable) \n");
        printf("  -c : cryptographic erase \n");
        printf("  -e : security erase \n");
        printf("       0: %s \n", erase_str[i++]);
        printf("       1: %s \n", erase_str[i++]);
        printf("       2: %s \n", erase_str[i++]);
        printf("       3: %s \n", erase_str[i++]);
        printf("       4: %s \n", erase_str[i++]);
        printf("       5: %s \n", erase_str[i++]);
        printf("       6: %s \n", erase_str[i++]);
        printf("       7: %s \n", erase_str[i++]);
        printf("       8: %s \n", erase_str[i++]);
        printf("       9: %s \n", erase_str[i++]);
        printf("  -g : get MK erase count \n");
        printf("  -s : set LBA via HPA    \n");
        printf("  -r : OSR test           \n");
    #else
        printf("Syntax: %s [-option ...]   \n", name);
        printf("Sample: %s -d /dev/sdb     \n", name);    
        printf("Sample: %s -d /dev/nvme0n1 \n", name);    
        printf("Option:\n");
        printf("  -d : device         \n");
        printf("  -a : iAnalyzer (0:Disable 1:Enable 2:Clean) \n");
        printf("  -w : write protect (0:Disable 1:Enable) \n");
        printf("  -c : cryptographic erase \n");
        printf("  -e : security erase \n");
        printf("       0: %s \n", erase_str[i++]);
        printf("       1: %s \n", erase_str[i++]);
        printf("       2: %s \n", erase_str[i++]);
        printf("       3: %s \n", erase_str[i++]);
        printf("       4: %s \n", erase_str[i++]);
        printf("       5: %s \n", erase_str[i++]);
        printf("       6: %s \n", erase_str[i++]);
        printf("       7: %s \n", erase_str[i++]);
        printf("       8: %s \n", erase_str[i++]);
        printf("       9: %s \n", erase_str[i++]);
        printf("  -g : get MK erase count \n");
        printf("  -s : set LBA via HPA    \n");
        printf("  -r : OSR test           \n");
        
        #if defined(OS_SOLARIS)
            printf("  -t : SE timeout (sec) \n");
        #endif

        printf("  -v : get VMware vSphere host SMART               \n");
        printf("     : 0: 3ME, 3IE3, InnoLite, 3MG2-P, 3MG-P & 3SE \n");
        printf("     : 1: 3ME & 3IE3 (SRR)                         \n");
    #endif
}

/*-----------------------------------------------------------------------------------------------------------*/
static char* rtrim(char *s)
{
    /* remove right whilt space of string */
    size_t len;
    char *cur;

    if(s && *s) 
    {
        len = strlen(s);
        cur = s + len - 1;

        while(cur != s && isspace(*cur))
            --cur, --len;

        cur[isspace(*cur) ? 0 : 1] = '\0';
    }

    return s;
}

/*-----------------------------------------------------------------------------------------------------------*/
static void show_info(DISK_INFO disk_info)
{
    DEV_INFO      dev      = disk_info.dev;
    IDENTIFY_INFO id       = disk_info.id;
    FEATURE_INFO  feature  = disk_info.id.feature;
    ANALYZER_INFO analyzer = disk_info.analyzer;

    printf("Model Name      : %s \n", id.ModelName);

    (strlen(id.CustSN) > 0) ? printf("Serial Num      : %s (%s) \n", id.SerialNum, id.CustSN)
                            : printf("Serial Num      : %s      \n", id.SerialNum);

    printf("FW Version      : %s \n", id.FWVer);    
    printf("WWN / EUI64     : %s \n", id.EUI64_WWN);

    (dev.OSR) ? printf("OSR             : YES \n") : printf("OSR             : NO \n");

    printf("------------------------------------------------------------------------\n");
    printf("BusType         : %d        \n", dev.BusType);
    printf("Capacity        : %.2f GB   \n", dev.Capacity);

    (feature.HPA) ? printf("LBAs / Native   : %lld / %lld \n", dev.LBAs, dev.NativeLBAs)
                  : printf("LBAs            : %lld        \n", dev.LBAs);

    if(dev.Health >= 0)        printf("Health          : %.2f    \n", dev.Health);
    if(dev.PECycle >= 0)       printf("PE Cycle        : %d      \n", dev.PECycle);
    if(dev.AvgErase >= 0)      printf("AVG. Erase      : %d      \n", dev.AvgErase);
    if(feature.TSensor)        printf("Temperature     : %d      \n", dev.Temperature);
    if(dev.PowerOnHours >= 0)  printf("Power ON Hours  : %d      \n", dev.PowerOnHours);
    if(dev.PowerOnCycles >= 0) printf("Power ON Cycle  : %d      \n", dev.PowerOnCycles);    

    (feature.LBA48)    ? printf("LBA48           : YES \n") : printf("LBA48           : NO \n");
    (feature.HPA)      ? printf("HPA             : YES \n") : printf("HPA             : NO \n");
    (feature.NCQ)      ? printf("NCQ             : YES \n") : printf("NCQ             : NO \n");     
    (feature.TRIM)     ? printf("TRIM            : YES \n") : printf("TRIM            : NO \n");     
    (feature.Security) ? printf("Security        : YES \n") : printf("Security        : NO \n");
    (feature.TSensor)  ? printf("TSensor         : YES \n") : printf("TSensor         : NO \n");
    (feature.Crypto)   ? printf("Crypto Erase    : YES \n") : printf("Crypto Erase    : NO \n");

    printf("Transfer        : %s \n", feature.TransferMode);
    printf("Interface       : %s \n", feature.Interface);
    printf("Standard        : %s \n", feature.Standard);

    switch(feature.Analyzer)
    {
        case 0:  printf("Analyzer        : YES (Disable) Ver %d \n", analyzer.ver); break;
        case 1:  printf("Analyzer        : YES (Enable) Ver %d \n",  analyzer.ver); break;
        default: printf("Analyzer        : NO  \n");                                break;
    }

    switch(feature.WProtect)
    {
        case 0:  printf("Write Protect   : YES (Disable) \n"); break;
        case 1:  printf("Write Protect   : YES (Enable) \n");  break;
        default: printf("Write Protect   : NO  \n");           break;
    }

    if(feature.InnoRobust) 
        printf("InnoRobust      : Yes \n");
    else
    {                   
        printf("InnoRobust      : NO \n");
        (feature.QErase) ? printf("QErase          : Yes \n") : printf("QErase          : NO \n");
    }    
}

/*-----------------------------------------------------------------------------------------------------------*/
static void show_smart(DISK_INFO disk_info)
{
    ATTRIB_INFO *attrib = disk_info.smart.attrib;
    ATTRIB_INFO *health = disk_info.smart.health;

    int i;

    for(i=0; i<disk_info.smart.attrib_cnt; i++)
    {
        if(i == 0)
        {
            printf("------------------------------------------------------------------------\n");
            printf("%-10s %-23s %-11s %s\n", " ID", "RAW Value", "Value", "SMART Attributes");
            printf("------------------------------------------------------------------------\n");
        }

        printf("[%02X] [%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X] [%12lld] %s\n",             
               attrib[i].id,     attrib[i].raw[1], attrib[i].raw[2],  attrib[i].raw[3],
               attrib[i].raw[4], attrib[i].raw[5], attrib[i].raw[6],  attrib[i].raw[7],
               attrib[i].raw[8], attrib[i].raw[9], attrib[i].raw[10], attrib[i].raw[11],
               (attrib[i].id == 0xC2) ? (int) attrib[i].val : (long long) attrib[i].val, 
               attrib[i].desc);
    }

    for(i=0; i<disk_info.smart.health_cnt; i++)
    {
        if(i == 0)
        {
            printf("------------------------------------------------------------------------\n");
            printf("Health Attribute                          Value\n");
            printf("------------------------------------------------------------------------\n");
        }

        if(i>=7 && i<=9)
        {
            printf("%s : %10lld %% \n", health[i].desc, (long long) health[i].val);
        }
        else
        {
            printf("%s : %10lld \n", health[i].desc, (long long) health[i].val);
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------*/
static void show_analyzer(DISK_INFO disk_info)
{
    ANALYZER_INFO analyzer = disk_info.analyzer;

    int i;

    #if !defined(SHOW_ANALYZER)
        return;
    #endif

    printf("\n------------------------------------------------------------------------\n");
    printf("Read & Write                      \n");
    printf("------------------------------------------------------------------------\n");
    printf("Sequential Read  = %d%%\t (%lu)\n", analyzer.TR_PCT[0], analyzer.TR_CNT[0]);
    printf("Random Read      = %d%%\t (%lu)\n", analyzer.TR_PCT[1], analyzer.TR_CNT[1]);
    printf("Sequential Write = %d%%\t (%lu)\n", analyzer.TW_PCT[0], analyzer.TW_CNT[0]);
    printf("Random Write     = %d%%\t (%lu)\n", analyzer.TW_PCT[1], analyzer.TW_CNT[1]);

    if(analyzer.ver == 0)
    {
        printf("------------------------------------------------------------------------\n");
        printf("Sequential Read \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_sequential; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_sequential_v0[i], 
                                                   analyzer.SR_PCT[i], analyzer.SR_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Sequential Write \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_sequential; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_sequential_v0[i],
                                                   analyzer.SW_PCT[i], analyzer.SW_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Random Read \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_random; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_random_v0[i],
                                                   analyzer.RR_PCT[i], analyzer.RR_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Random Write \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_random; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_random_v0[i],
                                                   analyzer.RW_PCT[i], analyzer.RW_CNT[i]);
    }
    else if(analyzer.ver == 1)
    {
        printf("------------------------------------------------------------------------\n");
        printf("Sequential Read \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_sequential; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_v1[i], 
                                                   analyzer.SR_PCT[i], analyzer.SR_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Sequential Write \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_sequential; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_v1[i], 
                                                   analyzer.SW_PCT[i], analyzer.SW_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Random Read \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_random; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_v1[i], 
                                                   analyzer.RR_PCT[i], analyzer.RR_CNT[i]);

        printf("------------------------------------------------------------------------\n");
        printf("Random Write \n");
        printf("------------------------------------------------------------------------\n");
        printf("Size \t Percentage\t Count\n");

        for(i=0; i<analyzer.cnt_random; i++)
            printf("%s\t %d%%\t         (%lu) \n", analyzer_v1[i], 
                                                   analyzer.RW_PCT[i], analyzer.RW_CNT[i]);
    }
}

/*-----------------------------------------------------------------------------------------------------------*/
#if defined(OS_WIN)
static void serase(int disk, int erase, int bus_type)
#elif defined(OS_SOLARIS)
static void serase(char *disk, int erase, int se_timeout, int bus_type)
#else
static void serase(char *disk, int erase, int bus_type)
#endif
{
    struct tm *timeinfo;
    time_t    now;

    int ret, status;

    #if defined(OS_WIN)
        umount_disk(disk);
    #endif

    time(&now);
    timeinfo = localtime(&now);

    printf("Erase Mode : %s        \n", erase_str[erase - SE_QERASE + 1]);
    printf("Start : %02d:%02d:%02d \n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fflush(stdout);                           

    /*------------------------------------------------------------*/
    ret = security_erase(disk, erase);

    if(!ret)
    {
        #if !defined(OS_SOLARIS)
            
            while(1)
            {
                status = (bus_type == 0x11) ? get_serase_status_nvme(disk) : get_serase_status(disk);

                time(&now);
                timeinfo = localtime(&now);

                printf("Erase : %02d:%02d:%02d %3d%%\r", timeinfo->tm_hour, timeinfo->tm_min, 
                                                         timeinfo->tm_sec,  status);

                fflush(stdout);          
            
                if(status >= 100)
                    break;  
            
                usleep(1000000);                           
            }   

        #else

            if(se_timeout < 0)
            {
                while(1)
                {
                    status = (bus_type == 0x11) ? get_serase_status_nvme(disk) : get_serase_status(disk);

                    time(&now);
                    timeinfo = localtime(&now);

                    printf("Erase : %02d:%02d:%02d %3d%%\r", timeinfo->tm_hour, timeinfo->tm_min, 
                                                             timeinfo->tm_sec,  status);

                    fflush(stdout);          
            
                    if(status >= 100)
                        break;  
            
                    usleep(1000000);                           
                }   
            }
            else
            {
                /* w/o se status */
                time(&now);
                timeinfo = localtime(&now);

                printf("Erase : %02d:%02d:%02d\r", timeinfo->tm_hour, timeinfo->tm_min, 
                                                   timeinfo->tm_sec);

                fflush(stdout);          
            }

        #endif
        
        printf("\nErase Done !! \n");
    }
    else
        printf("Erase Fail !! \n");

    #if defined(OS_WIN)
        mount_disk(disk, ' ');
    #endif
}

/*-----------------------------------------------------------------------------------------------------------*/
static int *log_mk_erase_cnt_encode(char *raw)
{
    int  encode[2] = {8, 7}; // encode_bin = encode[0] * raw_bin + encode[1]
    int  i;
    int *rtn = malloc(strlen(raw) * sizeof(int));

    for(i=0; i<strlen(raw); i++)
    {
        *(rtn + i) = *(raw + i) * encode[0] + encode[1];
    }
    return rtn;
}

/*-----------------------------------------------------------------------------------------------------------*/
void log_mk_erase_cnt_decode(char *log_file)
{
    int   encode[2] = {8, 7}; // encode_bin = encode[0] * raw_bin + encode[1]
    int   i, file_size;
    FILE *fp;
    int  *int_from_file;
    char *char_from_file;

    fp = fopen(log_file, "rb"); 
    if(fp)
    {
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);

        int_from_file = malloc(file_size * sizeof(int));
        char_from_file = malloc(file_size * sizeof(char));

        i = 0;
        while(!feof(fp))
        {
        fread(&int_from_file[i], sizeof(int), 1, fp);
        char_from_file[i] = (int_from_file[i] - encode[1])/ encode[0];
        i++;
        }
        char_from_file[i-1] = '\0';
        fclose(fp);

        printf("%s\n", char_from_file);
        free(int_from_file);
        free(char_from_file);
    }
}

/*-----------------------------------------------------------------------------------------------------------*/
static void erase_cnt_log(ERASE_CNT_INFO info, DISK_INFO disk)
{
#if 1   /* modify by yichen, 20191226
           store .bin instead of .log */
    FILE          *fp;
    unsigned long  i, j, k;
    char           log[64], str[128];
    int           *encode_ptr;

    memset(log, 0, sizeof(log));
    sprintf(log, "%s_%s.bin", rtrim(disk.id.FWVer), rtrim(disk.id.SerialNum));

    fp = fopen(log, "wb");

    if(fp)
    {
        //printf("------------------------------------------------------------------------\n");
        //printf("Total block unit : %ld \n", info.blk_unit);

        for(i=0, j=0; i<info.blk_unit; i++, j+=4)
        {
            memset(str, 0, sizeof(str));

            if(info.erase_cnt[i] < 0)
            {
                sprintf(str, "%ld\tx\t%02X %02X %02X %02X\n", 
                        i, info.raw[j+0], info.raw[j+1], info.raw[j+2], info.raw[j+3]);

            }
            else
            {
                sprintf(str, "%ld\t%ld\t%02X %02X %02X %02X\n", 
                        i, info.erase_cnt[i], info.raw[j+0], info.raw[j+1], info.raw[j+2], info.raw[j+3]);
            }

            encode_ptr = log_mk_erase_cnt_encode(str);
            for(k=0; k<strlen(str); k++)
            {
                fwrite(&encode_ptr[k], sizeof(int), 1, fp);
            }
            encode_ptr = NULL;
        }

        /* TableEraseCnt, UserEraseCnt, AvgTableEraseCnt, AvgUserEraseCnt, UserBlockCnt*/
        memset(str, 0, sizeof(str));
        sprintf(str, "TableEraseCnt\t%ld\n", info.TableEraseCnt);
        encode_ptr = log_mk_erase_cnt_encode(str);
        for(k=0; k<strlen(str); k++)
        {
            fwrite(&encode_ptr[k], sizeof(int), 1, fp);
        }
        encode_ptr = NULL;

        memset(str, 0, sizeof(str));
        sprintf(str, "UserEraseCnt\t%ld\n", info.UserEraseCnt);
        encode_ptr = log_mk_erase_cnt_encode(str);
        for(k=0; k<strlen(str); k++)
        {
            fwrite(&encode_ptr[k], sizeof(int), 1, fp);
        }
        encode_ptr = NULL;

        memset(str, 0, sizeof(str));
        sprintf(str, "AvgTableEraseCnt\t%ld\n", info.AvgTableEraseCnt);
        encode_ptr = log_mk_erase_cnt_encode(str);
        for(k=0; k<strlen(str); k++)
        {
            fwrite(&encode_ptr[k], sizeof(int), 1, fp);
        }
        encode_ptr = NULL;

        memset(str, 0, sizeof(str));
        sprintf(str, "AvgUserEraseCnt\t%ld\n", info.AvgUserEraseCnt);
        encode_ptr = log_mk_erase_cnt_encode(str);
        for(k=0; k<strlen(str); k++)
        {
            fwrite(&encode_ptr[k], sizeof(int), 1, fp);
        }
        encode_ptr = NULL;

        memset(str, 0, sizeof(str));
        sprintf(str, "UserBlockCnt\t%ld\n", info.UserBlockCnt);
        encode_ptr = log_mk_erase_cnt_encode(str);
        for(k=0; k<strlen(str); k++)
        {
            fwrite(&encode_ptr[k], sizeof(int), 1, fp);
        }
        encode_ptr = NULL;

        fclose(fp);

        // log_mk_erase_cnt_decode(log); /* for decoding log_mk_erase_cnt.bin */

        printf("\n%s saved !! \n", log);
    }
    else
        printf("Save %s fail !! \n", log);
#else
    FILE          *fp;
    unsigned long i, j;
    char          log[64], str[128];

    memset(log, 0, sizeof(log));
    sprintf(log, "%s_%s.log", rtrim(disk.id.FWVer), rtrim(disk.id.SerialNum));

    fp = fopen(log, "w+");

    if(fp)
    {
        //printf("------------------------------------------------------------------------\n");
        //printf("Total block unit : %ld \n", info.blk_unit);

        for(i=0, j=0; i<info.blk_unit; i++, j+=4)
        {
            memset(str, 0, sizeof(str));

            if(info.erase_cnt[i] < 0)
            {
                sprintf(str, "%ld\tx\t%02X %02X %02X %02X\n", 
                        i, info.raw[j+0], info.raw[j+1], info.raw[j+2], info.raw[j+3]);

            }
            else
            {
                sprintf(str, "%ld\t%ld\t%02X %02X %02X %02X\n", 
                        i, info.erase_cnt[i], info.raw[j+0], info.raw[j+1], info.raw[j+2], info.raw[j+3]);
            }

            fwrite(str, 1, strlen(str), fp);
        }

        /* TableEraseCnt, UserEraseCnt, AvgTableEraseCnt, AvgUserEraseCnt, UserBlockCnt*/
        memset(str, 0, sizeof(str));
        sprintf(str, "TableEraseCnt\t%ld\n", info.TableEraseCnt);
        fwrite(str, 1, strlen(str), fp);

        memset(str, 0, sizeof(str));
        sprintf(str, "UserEraseCnt\t%ld\n", info.UserEraseCnt);
        fwrite(str, 1, strlen(str), fp);

        memset(str, 0, sizeof(str));
        sprintf(str, "AvgTableEraseCnt\t%ld\n", info.AvgTableEraseCnt);
        fwrite(str, 1, strlen(str), fp);

        memset(str, 0, sizeof(str));
        sprintf(str, "AvgUserEraseCnt\t%ld\n", info.AvgUserEraseCnt);
        fwrite(str, 1, strlen(str), fp);

        memset(str, 0, sizeof(str));
        sprintf(str, "UserBlockCnt\t%ld\n", info.UserBlockCnt);
        fwrite(str, 1, strlen(str), fp);

        fclose(fp);

        printf("\n%s saved !! \n", log);
    }
    else
        printf("Save %s fail !! \n", log);
#endif
}

/*-----------------------------------------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    DISK_INFO      disk_info;
    FEATURE_INFO   feature;
    ERASE_CNT_INFO erase_cnt_info; /* for MK */

    #if defined(OS_WIN)
        int disk;        
    #else
        #if defined(OS_SOLARIS)
            int se_timeout = -1;
        #endif

        char disk[512] = { 0 };
    #endif

    bool ret;
    int  i, opt, mode = MODE_NONE;

    int  analyzer_status  = -1;
    int  wp_status        = -1;
    int  erase            = -1;
    bool bCryptographic   = false;
    bool bEraseCnt        = false;
    bool bOSRTest         = false;

    unsigned long NativeLBAs = -1;


    #if defined(OS_WIN)
    #else
        if(getuid())
        {
            show_usage(argv[0], true);
            printf("Permission denied !!\nYou are not logged as root.\n\n");
            return 0;
        }
    #endif

    if(argc < 3)
    {
        show_usage(argv[0], false);
        return 0;      
    }    

    while((opt = getopt_long(argc, argv, "?hd:a:w:v:e:ct:s:gr", long_opt, NULL)) != -1)
    {
        switch(opt)
        {
            case 'd':
                        #if defined(OS_WIN)
                            disk = atoi(optarg);
                        #else
                            strcpy(disk, optarg);
                        #endif
                        break;
            case 'a':
                        if(optarg[0] == '0')      analyzer_status = 0;
                        else if(optarg[0] == '1') analyzer_status = 1;
                        else if(optarg[0] == '2') analyzer_status = 2;
                        break;
            case 'w':
                        if(optarg[0] == '0')      wp_status = 0;
                        else if(optarg[0] == '1') wp_status = 1;
                        break;
            case 'v':
                        mode = atoi(optarg);
                        break;
            case 'c':
                        bCryptographic = true;
                        break;
            case 'g':
                        bEraseCnt = true;
                        break;
            case 's':
                        NativeLBAs = atoi(optarg);
                        break;
            case 'r':
                        bOSRTest = true;
                        break;
            case 'e':
                        erase = atoi(optarg) + SE_DESTROY;

                        if(erase < SE_DESTROY || erase >= SE_NONE)
                        {
                            show_usage(argv[0], false);
                            return 0;
                        }
                        break;
            #if defined(OS_SOLARIS)
                case 't':
                            se_timeout = atoi(optarg);
                            break;
            #endif
            case 'h':
            case '?':
            default:
                        show_usage(argv[0], false);
                        return 0;      
        }
    }

    if(!bOSRTest)    
        show_usage(argv[0], true);

    /*------------------------------------------------------------*/
    if(bCryptographic)
    {
        #if defined(OS_WIN)
            umount_disk(disk);
        #endif

        if(!crypto_erase(disk))
            printf("Cryptographic erase done !! \n");
        else
            printf("Cryptographic erase fail !! \n");

        #if defined(OS_WIN)
            mount_disk(disk, ' ');
        #endif
    }
    else if(erase >= SE_DESTROY && erase < SE_NONE)
    {
        ret = get_dev_info(disk, &disk_info);

        if(!ret)
        {
            /* security erase */
            #if defined(OS_SOLARIS)
                set_se_timeout(se_timeout);
                serase(disk, erase, se_timeout, disk_info.dev.BusType);
            #else
                serase(disk, erase, disk_info.dev.BusType);
            #endif
        }
    }
    else
    {
        if(mode == MODE_NONE)
        {
            /* set iAnalyzer state */
            if(analyzer_status != -1)
                set_analyzer(disk, analyzer_status);

            /* set write protect */
            if(wp_status != -1)
                set_write_protect(disk, wp_status);

            if(NativeLBAs != -1)
                set_native_lba(disk, NativeLBAs);

            ret = get_dev_info(disk, &disk_info);
        }
        else
            ret = get_vmware_smart(disk, &disk_info, mode);

        /*------------------------------------------------------------*/
        if(!ret)
        {
            feature = disk_info.id.feature;

            if(mode == MODE_NONE)
            {
                if(disk_info.isRAID)
                {
                    for(i=0; i<disk_info.raid_cnt; i++)
                    {
                        if(!disk_info.raid_status[i])
                            continue;

                        printf("************************************************************************\n");
                        printf("* CSMI : %d                                                             *\n", i);
                        printf("************************************************************************\n");

                        disk_info.id    = disk_info.raid_id[i];
                        disk_info.smart = disk_info.raid_smart[i];
                        disk_info.dev   = disk_info.raid_dev[i];

                

                        show_info(disk_info);
                        show_smart(disk_info);
                    }                    
                }
                else
                {
                    /* OSR test */
                    if(disk_info.dev.OSR && bOSRTest)
                        OSRTest(disk, disk_info);
                    else
                    {
                        show_info(disk_info);
                        show_smart(disk_info);
    
                        if(feature.Analyzer == 1)
                            show_analyzer(disk_info);
                    }
                }
            }
            else
                show_smart(disk_info);

            /* attributes release */
            attrib_release(disk_info.smart.attrib, disk_info.smart.health);

            /* for 3TE7 MK only */
            if(bEraseCnt)
            {
                if(strstr(disk_info.id.FWVer, "S18927") || strstr(disk_info.id.FWVer, "S18B28")                                        
                || strstr(disk_info.id.FWVer, "S19114"))
                {
                    memset(&erase_cnt_info, 0, sizeof(ERASE_CNT_INFO));

                    if(!get_erase_cnt(disk, &erase_cnt_info))
                        erase_cnt_log(erase_cnt_info, disk_info);
                    else
                        printf("get_erase_cnt fail !! \n");
                }
            }
        }
        else
            printf("get_dev_info() NG \n");
    }

    return 0;
}
