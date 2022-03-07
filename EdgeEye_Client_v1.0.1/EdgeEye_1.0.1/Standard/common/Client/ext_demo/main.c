#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "libiCAPClient.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"

char onExit = 0;
int cport_nr = 3;

#ifdef WIN32
BOOL WINAPI gotExitCmd(DWORD dwType)
{
    switch(dwType) {
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			onExit = 1;
			break;
    }
    return TRUE;
}
#else
void gotExitCmd(int sig)
{
	onExit = 1;
}
#endif

double GetValue()
{
  int n;
  double ret = 0;
  unsigned char buf[4096];

  RS232_cputs(cport_nr, "r");

  n = RS232_PollComport(cport_nr, buf, 4095);

  if(n > 0)
  {
    buf[n] = 0;

    ret = atof(buf);
  }

  return ret;
}

int ControlLED(void* str)
{
  if(strncmp((char*)str, "on", 2))
  {
    RS232_cputs(cport_nr, "0");
    return 0;
  }
  else if(strncmp((char*)str, "off", 3))
  {
    RS232_cputs(cport_nr, "1");
    return 0;
  }

  return 1;
}

int main()
{
  int ret = 0, bdrate=115200;

  char mode[]={'8','N','1',0};
  
  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");

    return(0);
  }

#ifdef WIN32
	if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)gotExitCmd,TRUE))
	{
		printf("Fail to listen Ctrl-C signal!\n");
	}
#else
	(void)signal(SIGINT, gotExitCmd);
#endif

#if 0
  while(1)
  {
    ControlLED((void*)"on");

#ifdef _WIN32
    Sleep(1000);
#else
    usleep(1000000);  /* sleep for 1 Second */
#endif
    ControlLED((void*)"off");

    #ifdef _WIN32
    Sleep(1000);
#else
    usleep(1000000);  /* sleep for 1 Second */
#endif

    printf("Get voltage: %f\n", GetValue());
  }
#endif

  if(iCAP_Connect() == 0)
  {

    ret = iCAP_GetClientStatus();
    printf("Device servcie status :%d\n", ret);

    //Add external sensor
    iCAP_AddExternalSensor("Voltage", "V", 0, GetValue);

    //Add remote device
    iCAP_AddRemoteDevice("LED", "onoff", 1, ControlLED);

    while(onExit == 0)
    {
      usleep(10000L);
    }

    //Remove external sensor
    iCAP_RemoveExternalSensor("Voltage");

    //Remove remote device
    iCAP_RemoveRemoteDevice("LED");

  }

  return(0);
}