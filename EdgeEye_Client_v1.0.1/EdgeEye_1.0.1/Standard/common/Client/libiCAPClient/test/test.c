#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "DeviceAgent.h"

#ifdef WIN32
#define PASS "  PASS"
#else
#define PASS "  [\x1b[32mPASS\x1b[0m]"
#endif

double GetSensorValue(void)
{
	return 99.9;
}

int Control(void* cmd)
{
	return strcmp("test cmd", cmd);
}

int main(int argc, char** argv)
{
	assert((DA_AddExtSensorToList("a", GetSensorValue) == 0));
	printf(PASS"AddExtSensorToList(\"a\", GetSensorValue)\n");
	assert((DA_GetExtSensorValue("a") == 99.9));
	printf(PASS"GetExtSensorValue(\"a\")\n");
	assert((DA_RemoveExtSensor("a") == 0));
	printf(PASS"RemoveExtSensor(\"a\")\n");
	assert((DA_AddRemoteDeviceToList("a", Control) == 0));
	printf(PASS"AddRemoteDeviceToList(\"a\", GetSensorValue)\n");
	assert((DA_SendRemoteDeviceCmd("a", (void*)"test cmd") == 0));
	printf(PASS"SendRemoteDeviceCmd(\"a\", (void*)\"test cmd\"))\n");
	assert((DA_RemoveRemoteDevice("a") == 0));
	printf(PASS"RemoveRemoteDevice(\"a\")\n");
	return 0;
}
