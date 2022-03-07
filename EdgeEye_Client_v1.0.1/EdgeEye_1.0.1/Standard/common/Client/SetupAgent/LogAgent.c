#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "LogAgent.h"

void WriteToFile(char* logstr)
{
	FILE* fp = NULL;
	char* currentTimeStr;
	time_t timer;
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	currentTimeStr = (char*)malloc(sizeof(char) * 30);

	strftime(currentTimeStr, 30, "%Y-%m-%d %H:%M:%S", tm_info);

	fp = fopen(LOG_FILE_NAME, "a");

	if(fp)
	{
		fprintf(fp, "[%s]%s\n", currentTimeStr, logstr);

		fclose(fp);
	}

	free(currentTimeStr);
}

void LogAgent_Write(char* formatStr, ...)
{
	va_list vaptr;
	int ret;
	char* logstr = (char*)malloc(sizeof(char) * MAX_LOG_STR_SIZE);

	va_start(vaptr, formatStr);
	ret = vsprintf(logstr, formatStr, vaptr);
	va_end(vaptr);

	if(ret > 0)
	{
		WriteToFile(logstr);
	}

	free(logstr);
}