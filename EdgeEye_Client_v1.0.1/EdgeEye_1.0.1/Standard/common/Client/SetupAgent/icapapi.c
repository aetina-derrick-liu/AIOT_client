#include <stdio.h>
#include "icapapi.h"
#include "JsonParser.h"
#include "LogAgent.h"
#include "ServiceCtl.h"

/* return == 0: success, return != 0: failed */
/*----------------------------------------------------------------*/
int iCAPReCnnt (API_INFO info)
{
	struct json_object* jobj_write;
	char* write_str = (char*)malloc(sizeof(char) * 512);
	FILE* fp;
	int ret;

	if(write_str == NULL)
	{
		LogAgent_Write("Failed to allocate memory for write_str.");
		return 1;
	}

	jobj_write = json_object_new_object();

	json_object_object_add(jobj_write, "ServerAddr", json_object_new_string(info.ipAddr));
	json_object_object_add(jobj_write, "Long", json_object_new_double(info.lo));
	json_object_object_add(jobj_write, "Lati", json_object_new_double(info.la));
	json_object_object_add(jobj_write, "Interval", json_object_new_int(info.itvl));

	write_str = (char*)json_object_to_json_string(jobj_write);

	fp = fopen(SETTING_FILE_PATH, "w");

	if(fp == NULL)
	{
		LogAgent_Write("Failed to open setting file.");
		return 1;
	}

	fprintf(fp, "%s\n", write_str);

	fclose(fp);

	free(write_str);

	json_object_put(jobj_write);

	ret = ServiceStop();

	if(ret != 0)
	{
		LogAgent_Write("Failed to stop service.");
	}

	ret = ServiceStart();

	if(ret != 0)
	{
		LogAgent_Write("Failed to start service.");
		return 1;
	}

	return 0;

} /* END: iCAPReCnnt() */