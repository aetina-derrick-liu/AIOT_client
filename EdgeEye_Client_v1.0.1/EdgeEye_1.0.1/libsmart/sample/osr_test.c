#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include "libsmart.h"

enum
{
    KEY_EXIT    = 0,
    KEY_CLEAN,
    KEY_SET_CP_LBA,
    KEY_SET_RESERVED,
    KEY_BACKUP,
    KEY_RECOVERY,
};

/*-----------------------------------------------------------------------------------------------------------*/
static char get_key(char *str, bool wait)
{
    static int c;

    printf("%s", str);
    fflush(stdout);
    scanf("%d", &c);
    
    if(wait)
        getchar(); 
        
    clearerr(stdin);

    return c;   
}

/*-----------------------------------------------------------------------------------------------------------*/
#if defined(OS_WIN)
extern void menu(int disk, DISK_INFO info, OSR_INFO *osr)
#else
extern void menu(char *disk, DISK_INFO info, OSR_INFO *osr)
#endif
{
    OSR_STATUS *s;
    bool       isBackup;
    bool       isSetCopyLBA;
    bool       isBackupEnd;
    int        i;

START:

    fflush(stdout);

    system("clear");
    printf("************************************************************************\n");
    printf("* Innodisk iSMART for OSR %s                           %s *\n", get_lib_ver(), get_build_date());
    printf("************************************************************************\n");
    printf("Model Name : %s \n", info.id.ModelName);
    printf("Serial Num : %s \n", info.id.SerialNum);
    printf("FW Version : %s \n", info.id.FWVer);   
    printf("========================================================================\n");

    /* cheak status */
    memset(osr, 0, sizeof(OSR_INFO));
    execOSR(disk, OSR_CHECK_STATUS, osr);

    s = (OSR_STATUS *) osr->status;

    isSetCopyLBA = s->isSetCopyLBA;
    isBackupEnd  = s->isBackupEnd;

    printf("Set copy LBA           : %d \n\n", s->isSetCopyLBA);

    printf("Backup end             : %d \n", s->isBackupEnd);
    printf("Backup write fail      : %d \n", s->isBackupWriteFail);
    printf("Backup read ECC fail   : %d \n", s->isBackupReadECCFail);
    printf("Backup start           : %d \n", s->isBackupStart);

    printf("Recovery end           : %d \n", s->isRecoveryEnd);
    printf("Recovery write fail    : %d \n", s->isRecoveryWriteFail);
    printf("Recovery read ECC fail : %d \n", s->isRecoveryReadECCFail);
    printf("Recovery start         : %d \n", s->isRecoveryStart);

    printf("\nHidden area            : %d \n", s->isSetHidden);

    if(s->isSetHidden)
        printf("Hidden size            : %d GB \n", s->hidden_size);

    printf("Copy LBA start         : %d \n", s->copy_lba_start);
    printf("Copy LBA end           : %d \n", s->copy_lba_end);
    printf("Target LBA end         : %d \n", s->target_lba_start);
    printf("Write protect start    : %d \n", s->write_protect_start);
    printf("Write protect end      : %d \n", s->write_protect_end);
    printf("reserved size sector   : %d \n", s->reserved_size_sector);

    /* backup or recovery */    
    if(s->isBackupStart || s->isRecoveryStart)
    {
        isBackup = (s->isBackupStart) ? true : false;        

        (isBackup) ? execOSR(disk, OSR_BACKUP,   osr) 
                   : execOSR(disk, OSR_RECOVERY, osr);

        while(1)
        {
            memset(osr, 0, sizeof(OSR_INFO));
            execOSR(disk, OSR_CHECK_STATUS, osr);

            s = (OSR_STATUS *) osr->status;

            if((isBackup && s->isBackupEnd) || (!isBackup && s->isRecoveryEnd))
                goto START;

            #if 0
                printf(".");
            #else
                (isBackup) ? printf("Backup : %d%% \r",   s->back_recv_pct) 
                           : printf("Recovery : %d%% \r", s->back_recv_pct);
            #endif

            fflush(stdout);
            usleep(1000000);            
        }
    }

    /* read partition */
    memset(osr, 0, sizeof(OSR_INFO));
    execOSR(disk, OSR_READ_PTN, osr);

    printf("\nPartition : %d \n", osr->n);

    for(i=0; i<osr->n; i++)
    {
        printf("%d (%lld GB) : %lld ~ %lld \n", i+1, 
                                                (osr->ptn[i].tail - osr->ptn[i].head + 1) * 512 / 1024 / 1024 / 1024,
                                                osr->ptn[i].head, osr->ptn[i].tail);
    }
     
    printf("\n========================================================================\n");
    printf("1. Clean status        \n");

    if(!isSetCopyLBA)
    {
        printf("2. Set copy LBA        \n");
        printf("3. Set reserved sector \n");
    }
    else
    {
        printf("4. Backup              \n");

        if(isBackupEnd)
            printf("5. Recovery            \n");
    }

    printf("0. Exit                \n");
}

/*-----------------------------------------------------------------------------------------------------------*/
#if defined(OS_WIN)
extern void set_cp_lba(int disk, OSR_INFO *osr)
#else
extern void set_cp_lba(char *disk, OSR_INFO *osr)
#endif
{
    int n, ret = 0;

    if(osr->n > 0)
    {
        while(1)
        {
            if((n = get_key("Pleease select partition : ", false)) == '0')        
                return;

            if(n > 0 && n <= osr->n)
                break;
        }    

        osr->rev_cp_sector = osr->ptn[n-1].tail;   
        ret = execOSR(disk, OSR_SET_COPY_LBA, osr);

        if(ret)
        {
            switch(ret)
            {
                case ERR_ENABLED:
                        printf("Set copy LBA already !! \n"); 
                        break;
                case ERR_4K_ALIGN:         
                        printf("LBA is not 4K alignment !! \n"); 
                        break;
                case ERR_NOT_ENOUGH_SPACE: 
                        printf("Not enough space !! \n");       
                        break;
            }

            fflush(stdout);
            system("pause");
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------*/
#if defined(OS_WIN)
extern void OSRTest(int disk, DISK_INFO info)
#else
extern void OSRTest(char *disk, DISK_INFO info)
#endif
{
    OSR_INFO osr;
    int      n;

    #if !defined(OS_WIN)
        sync();
    #endif

    menu(disk, info, &osr);

    while(1)
    {       
        if((n = get_key("\nPleease select: ", true)) == 0)        
            break;  
            
        switch(n)
        {
            case KEY_CLEAN:
                                    /* clean status */
                                    memset(&osr, 0, sizeof(OSR_INFO));
                                    execOSR(disk, OSR_CLEAN_STATUS, &osr);
                                    break;
            case KEY_SET_CP_LBA:
                                    /* set copy lba */
                                    set_cp_lba(disk, &osr);
                                    break;
            case KEY_SET_RESERVED:
                                    /* set reserved sector */
                                    memset(&osr, 0, sizeof(OSR_INFO));
                                    osr.rev_cp_sector = 2560;
                                    execOSR(disk, OSR_SET_RSV_SECTOR, &osr);        
                                    break;
            case KEY_BACKUP:
                                    /* backup */
                                    #if !defined(OS_WIN)
                                        sync();
                                    #endif

                                    memset(&osr, 0, sizeof(OSR_INFO));
                                    execOSR(disk, OSR_BACKUP, &osr);                                                                           
                                    break;
            case KEY_RECOVERY:
                                    /* recovery */
                                    memset(&osr, 0, sizeof(OSR_INFO));
                                    execOSR(disk, OSR_RECOVERY, &osr);      
                                    break;
            default:
                                    break;
        }            
        
        menu(disk, info, &osr);
    }
}
