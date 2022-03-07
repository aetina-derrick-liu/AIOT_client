#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "EApi.h"
#include "EAPIInfo.h"

char* lib_SysInfo_GetEApiInfo()
{
	EApiStatus_t eapistatus = 0;
	char* ret = (char*)malloc(sizeof(char) * 1024);
	
	memset(ret, 0, sizeof(char) * 1024);
	
	if ((eapistatus = EApiLibInitialize()) != EAPI_STATUS_SUCCESS) {
		printf("EApiLibInitialize error ... error code: %X\n", eapistatus);
		return ret;
	}

	uint32_t buffer_size = 1024;
	if ((eapistatus = EApiInformationOut2JSONStringA(ret, &buffer_size)) != EAPI_STATUS_SUCCESS)
		printf("EApiInformationOut2JSONStringA error ... error code: %X\n", eapistatus);

#ifdef DEBUG
	printf("Data : \n%s\n", ret);
#endif

	if ((eapistatus = EApiLibUnInitialize()) != EAPI_STATUS_SUCCESS)
		printf("EApiLibUnInitialize error ... error code: %X\n", eapistatus);

	return ret;
}
#endif