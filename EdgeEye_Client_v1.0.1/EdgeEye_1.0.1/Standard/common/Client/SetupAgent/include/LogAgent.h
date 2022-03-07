#ifndef __LOGAGENT_H__
#define __LOGAGENT_H__

#define MAX_LOG_STR_SIZE 512

#define LOG_FILE_NAME "/Users/All Users/Innodisk/iCAP_ClientService/ProgramLog.log"

void LogAgent_Write(char* formatStr, ...);

#endif
