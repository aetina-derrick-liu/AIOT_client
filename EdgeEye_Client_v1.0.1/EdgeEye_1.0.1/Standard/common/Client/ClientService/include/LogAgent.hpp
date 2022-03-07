#ifndef __LOGAGENT_HPP__
#define __LOGAGENT_HPP__

#define MAX_LOG_STR_SIZE 8192
#ifdef WIN32
    #ifdef iON
        #define LOG_FILE_NAME "iONetworks/EZ_SSD_SOS/ServiceLog.log"
    #else
        #define LOG_FILE_NAME "Innodisk/iCAP_ClientService/ServiceLog.log"
    #endif
#else
#define LOG_FILE_NAME "/var/iCAP_Client/ServiceLog.log"
#endif

void LogAgent_Write(char* formatStr, ...);

#endif
