/* Allen 2017.11.24 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CMDParser.h"
#include "DisplayInfo.h"

DISPLAY_INFO* lib_SysInfo_GetDisplayInfo()
{
	DISPLAY_INFO* ret = (DISPLAY_INFO*)malloc(sizeof(DISPLAY_INFO));
#ifdef WIN32
	char** tmp = GetWMIData("ROOT\\CIMV2",
							"SELECT * FROM Win32_DisplayConfiguration",
							0,
							1,
							"DeviceName");
	ret->DeviceName = *(tmp+0);
	free(tmp);
#else
#endif
	return ret;
}
