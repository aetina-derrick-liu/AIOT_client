#ifndef __CMDPARSER_H__
#define __CMDPARSER_H__

#define MAX_CMD_STR_LEN 1024

char* GetCommandResult(char* ret, char* cmd, char* splitStr, int index);
char* MEMAllocting(const char* func);
char* MEMReallocting(char* inputStr);
char* GetJetsonResult(char* ret,char* item,int len);
char* GetJetsonResult_int(char* ret,char* item,int len);
#ifdef WIN32
char** GetWMIData(char* resourcestring, char* querystring, int queryIndex, int numberofquery, ...);
#endif

#endif
