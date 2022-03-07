#define WIN32_LEAN_AND_MEAN
#include <unistd.h>
#include <locale.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#endif
#include <cstdlib>
#include <signal.h>
#include "main.hpp"
#include "ClientService.hpp"
#include "LogAgent.hpp"
#include "DBInterface.hpp"

static char onExit = 0;
int ServiceStatus = 4;
int ReconnectCount = 0;

#define RECONNECT_TIMER 6000

#ifdef WIN32
  #ifdef DAEMON

    #define   SERVICE_NAME   "iCAP"
    #define   SVC_ERROR      1

    void ServiceMain    (int argc, char *argv[]);
    void SvcCtrlHandler (DWORD dwCtrl);
    void ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
    void SvcReportEvent (LPTSTR szFunction);

    HANDLE                  ghSvcStopEvent = NULL;
    SERVICE_STATUS          gSvcStatus; 
    SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
    SERVICE_TABLE_ENTRY     ServiceTable[2] =
    {
      {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
      {NULL, NULL}
    };

    void ServiceMain (int argc, char** argv)
    {
      int  rtn = -1;
      int wait_interval = 1000;

      /* Register the handler function for the service */
      gSvcStatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, (LPHANDLER_FUNCTION) SvcCtrlHandler);

      if( !gSvcStatusHandle )
      { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
        return;
      }

      /* These SERVICE_STATUS members remain as set here */
      gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
      gSvcStatus.dwServiceSpecificExitCode = 0;

      /* Report initial status to the SCM */
      ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

      /* Perform service-specific initialization and work */
      ghSvcStopEvent = CreateEvent( NULL,    /* default security attributes */
                                    true,    /* manual reset event */
                                    false,   /* not signaled */
                                    NULL);   /* no name */

      if ( ghSvcStopEvent == NULL)
      {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
      }

      /* Report running status when initialization is complete. */
      ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

      if(gSvcStatus.dwCurrentState == SERVICE_RUNNING)
      {
        remove(LOG_FILE_NAME);
      }

      /* iCAP api function here */
      ProgramMain();

      WaitForSingleObject(ghSvcStopEvent, INFINITE);
      ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    } 

    void SvcCtrlHandler (DWORD dwCtrl)
    {
      /* Handle the requested control code */

      switch(dwCtrl) 
      {  
        case SERVICE_CONTROL_STOP:
          onExit = 1;
          ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
          SetEvent(ghSvcStopEvent); /* Signal the service to stop */
          ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
          break;
        case SERVICE_CONTROL_INTERROGATE:
          break;
        case SERVICE_CONTROL_SHUTDOWN:
          break;
        default:
          break;
      }
    } 

    void ReportSvcStatus (DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
    {
      static DWORD dwCheckPoint = 1;

      /* Fill in the SERVICE_STATUS structure */
      gSvcStatus.dwCurrentState  = dwCurrentState;
      gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
      gSvcStatus.dwWaitHint      = dwWaitHint;

      if(dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
      else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

      if((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
      else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

      /* Report the status of the service to the SCM. */
      SetServiceStatus(gSvcStatusHandle, &gSvcStatus);


    } 

    void SvcReportEvent (LPTSTR szFunction) 
    { 
      HANDLE    hEventSource;
      LPCTSTR   lpszStrings[2];
      TCHAR     Buffer[80];

      hEventSource = RegisterEventSource(NULL, SERVICE_NAME);

      if(NULL != hEventSource)
      {
        sprintf(Buffer, "%s failed with %d", szFunction, (int) GetLastError());

        lpszStrings[0] = SERVICE_NAME;
        lpszStrings[1] = Buffer;

        ReportEvent( hEventSource,        /* event log handle */
                     EVENTLOG_ERROR_TYPE, /* event type */
                     0,                   /* event category */
                     SVC_ERROR,           /* event identifier */
                     NULL,                /* no security identifier */
                     2,                   /* size of lpszStrings array */
                     0,                   /* no binary data */
                     lpszStrings,         /* array of strings */
                     NULL);               /* no binary data */

        DeregisterEventSource(hEventSource);
      }
    }

    void SysServiceStartup()
    {
      StartServiceCtrlDispatcher(ServiceTable);
    }

  #else
    BOOL WINAPI gotExitCmd(DWORD dwType)
    {
        switch(dwType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
          onExit = 1;
          break;
        }
        return TRUE;
    }
  #endif
#else
  void gotExitCmd(int sig)
  {
  	onExit = 1;
  }
#endif

void DeviceLogout()
{
	onExit = 1;
}

void ServiceStartup()
{
  int ret;

  SA_Start(2888);

  ret = ClientService_Start();
}

void ServiceConnectionHandler()
{
  while(onExit == 0)
  {
    usleep(10000L);
    if(ServiceStatus != 0)
    {
      if(ReconnectCount >= RECONNECT_TIMER)
      {
#ifdef iON       
        LogAgent_Write((char*)"EZ_SSD_SOS Service was disconnected, try to reconnect.");
#else
        LogAgent_Write((char*)"iCAP Service was disconnected, try to reconnect.");
#endif
        ClientService_Start();
        ReconnectCount = 0;
      }
      else
      {
        ReconnectCount++;
      }
    }
    else
    {
      ReconnectCount = RECONNECT_TIMER;
    }
  }
}

void ProgramMain()
{
  ServiceStartup();

  ServiceConnectionHandler();

  ClientService_Stop();
}

int main(int argc, char** argv)
{
  signal(SIGILL, SIG_IGN);
  int return_value = 0;

  LogAgent_Write((char*)"Program start");
  setlocale(LC_ALL, "en_US.UTF-8");
  return_value = ClientService_Initialize();

  if(return_value != 0)
  {
    return -1;
  }

#ifdef WIN32
  _wsetlocale(LC_ALL, L"");
  #ifdef DAEMON
  	SysServiceStartup();
  #else
    SetConsoleCtrlHandler(gotExitCmd, TRUE);
    ProgramMain();
  #endif
#else
  (void)signal(SIGINT, gotExitCmd);

  ProgramMain();
#endif
	return 0;
}