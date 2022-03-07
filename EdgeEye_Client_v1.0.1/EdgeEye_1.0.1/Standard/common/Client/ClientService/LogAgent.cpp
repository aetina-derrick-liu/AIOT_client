#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "LogAgent.hpp"

void WriteToFile(char* logstr)
{
#ifdef DAEMON
	FILE* fp = NULL;
#endif
	char currentTimeStr[30]= {0};
	time_t timer;
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(currentTimeStr, 30, "%Y-%m-%d %H:%M:%S", tm_info);

#ifdef DAEMON
	fp = fopen(LOG_FILE_NAME, "a");

	if(fp)
#endif
	{
#ifdef DAEMON
		fprintf(fp, "[%s]%s\n", currentTimeStr, logstr);
		fclose(fp);
#else
		printf("[%s]%s\n", currentTimeStr, logstr);
#endif
	}
}

void LogAgent_Write(char* formatStr, ...)
{
	va_list vaptr;
	int ret;
	char logstr[MAX_LOG_STR_SIZE];

	va_start(vaptr, formatStr);
	ret = vsprintf(logstr, formatStr, vaptr);
	va_end(vaptr);

	if(ret > 0)
	{
		WriteToFile(logstr);
	}
}