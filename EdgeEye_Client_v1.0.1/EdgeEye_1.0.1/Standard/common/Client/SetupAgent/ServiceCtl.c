#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>
#include <windows.h>
#include "ServiceCtl.h"
#include "LogAgent.h"

#define SERVICE_NAME "iCAP"

int ServiceStop (void)
{
  DWORD                   dwWaitTime;
  DWORD                   dwBytesNeeded;
  DWORD                   dwTimeout = 30000; /* 30-second time-out */
  DWORD                   dwStartTime = GetTickCount();
  SERVICE_STATUS_PROCESS  ssp;
  SC_HANDLE               schSCManager;
  SC_HANDLE               schService;

  /* Get a handle to the SCM database */
  schSCManager = OpenSCManager(NULL,                    /* local computer */
                               NULL,                    /* ServicesActive database */
                               SC_MANAGER_ALL_ACCESS);  /* full access rights */

  if(NULL == schSCManager) 
  {
      LogAgent_Write("OpenSCManager failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }

  /* Get a handle to the service */
  schService = OpenService(schSCManager,
                           SERVICE_NAME,
                           SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);

  if (schService == NULL)
  {
      LogAgent_Write("OpenService failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }

  /* Make sure the service is not already stopped */
  if (!QueryServiceStatusEx(schService,
                            SC_STATUS_PROCESS_INFO,
                            (LPBYTE) &ssp,
                            sizeof(SERVICE_STATUS_PROCESS),
                            &dwBytesNeeded))
  {
      LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }

  if(ssp.dwCurrentState == SERVICE_STOPPED)
  {
    #ifndef DONT_SHOW_MSG
      printf("Service is already stopped.\n\n");
    #endif

    goto failed;
  }

  /* If a stop is pending, wait for it */
  while(ssp.dwCurrentState == SERVICE_STOP_PENDING)
  {
      LogAgent_Write("Service stop pending...\n\n");

    /* Do not wait longer than the wait hint. A good interval is
     * one-tenth of the wait hint but not less than 1 second
     * and not more than 10 seconds */

    dwWaitTime = ssp.dwWaitHint / 10;

    if(dwWaitTime < 1000)        dwWaitTime = 1000;
    else if(dwWaitTime > 10000)  dwWaitTime = 10000;

    Sleep(dwWaitTime);

    if (!QueryServiceStatusEx(schService,
                              SC_STATUS_PROCESS_INFO,
                              (LPBYTE) &ssp,
                              sizeof(SERVICE_STATUS_PROCESS),
                              &dwBytesNeeded))
    {
        LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());

      goto failed;
    }

    if(ssp.dwCurrentState == SERVICE_STOPPED)
    {
        LogAgent_Write("Service stopped successfully.\n\n");

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);
      return 0;
    }

    if(GetTickCount() - dwStartTime > dwTimeout)
    {
        LogAgent_Write("Service stop timed out.\n\n");

      goto failed;
    }
  }

  /* Send a stop code to the service */
  if(!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp))
  {
      LogAgent_Write( "ControlService failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }

  /* Wait for the service to stop */
  while (ssp.dwCurrentState != SERVICE_STOPPED)
  {
    Sleep(ssp.dwWaitHint);

    if(!QueryServiceStatusEx(schService,
                             SC_STATUS_PROCESS_INFO,
                             (LPBYTE) &ssp,
                             sizeof(SERVICE_STATUS_PROCESS),
                             &dwBytesNeeded))
    {
        LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());

      goto failed;
    }

    if(ssp.dwCurrentState == SERVICE_STOPPED)
      break;

    if(GetTickCount() - dwStartTime > dwTimeout)
    {
        LogAgent_Write("Wait timed out\n\n");

      goto failed;
    }
  }

  CloseServiceHandle(schService);
  CloseServiceHandle(schSCManager);
  return 0;

  failed:
          if(schService)   CloseServiceHandle(schService);
          if(schSCManager) CloseServiceHandle(schSCManager);
          return 1;

} /* END: ServiceStop() */

int ServiceStart (void)
{
  DWORD                   dwOldCheckPoint;
  DWORD                   dwStartTickCount;
  DWORD                   dwWaitTime;
  DWORD                   dwBytesNeeded;

  SC_HANDLE               schSCManager;
  SC_HANDLE               schService;
  SERVICE_STATUS_PROCESS  ssStatus;

  /* Get a handle to the SCM database. */
  schSCManager = OpenSCManager(NULL,                    /* local computer */
                               NULL,                    /* servicesActive database */
                               SC_MANAGER_ALL_ACCESS);  /* full access rights */

  if(schSCManager == NULL) 
  {
      LogAgent_Write("OpenSCManager failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }

  /* Get a handle to the service */
  schService = OpenService(schSCManager,         /* SCM database */
                           SERVICE_NAME,         /* name of service */
                           SERVICE_ALL_ACCESS);  /* full access */

  if (schService == NULL)
  {
      LogAgent_Write("OpenService failed (%d)\n\n", (int) GetLastError());
    
    goto failed;
  }

  /* Check the status in case the service is not stopped */

  if (!QueryServiceStatusEx(schService,                     /* handle to service */
                            SC_STATUS_PROCESS_INFO,         /* information level */
                            (LPBYTE) &ssStatus,             /* address of structure */
                            sizeof(SERVICE_STATUS_PROCESS), /* size of structure */
                            &dwBytesNeeded))                /* size needed if buffer is too small */
  {
      LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());


    goto failed;
  }

  /* Check if the service is already running. It would be possible 
   * to stop the service here, but for simplicity this example just returns */
  if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
  {
      LogAgent_Write("Cannot start the service because it is already running\n\n");

    goto failed;
  }

  /* Save the tick count and initial checkpoint. */
  dwStartTickCount = GetTickCount();
  dwOldCheckPoint  = ssStatus.dwCheckPoint;

  /* Wait for the service to stop before attempting to start it. */

  while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
  {
    /* Do not wait longer than the wait hint. A good interval is
     * one-tenth of the wait hint but not less than 1 second
     * and not more than 10 seconds */

    dwWaitTime = ssStatus.dwWaitHint / 10;

    if(dwWaitTime < 1000)        dwWaitTime = 1000;
    else if (dwWaitTime > 10000) dwWaitTime = 10000;

    Sleep(dwWaitTime);

    /* Check the status until the service is no longer stop pending */
    if (!QueryServiceStatusEx(schService,                     /* handle to service */
                              SC_STATUS_PROCESS_INFO,         /* information level */
                              (LPBYTE) &ssStatus,             /* address of structure */
                              sizeof(SERVICE_STATUS_PROCESS), /* size of structure */
                              &dwBytesNeeded))                /* size needed if buffer is too small */
    {
        LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());
      
      goto failed;
    }

    if (ssStatus.dwCheckPoint > dwOldCheckPoint)
    {
      /* Continue to wait and check. */
      dwStartTickCount = GetTickCount();
      dwOldCheckPoint  = ssStatus.dwCheckPoint;
    }
    else
    {
      if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
      {
          LogAgent_Write("Timeout waiting for service to stop\n\n");


        goto failed;
      }
    }
  }

  /* Attempt to start the service. */

  if (!StartService(schService,  /* handle to service */
                    0,           /* number of arguments */
                    NULL))       /* no arguments */
  {
      LogAgent_Write("StartService failed (%d)\n\n", (int) GetLastError());

    goto failed;
  }
  else
  {
      LogAgent_Write("Service start pending...\n\n");
  }


  /* Check the status until the service is no longer start pending */
  if (!QueryServiceStatusEx(schService,                     /* handle to service */
                            SC_STATUS_PROCESS_INFO,         /* info level */
                            (LPBYTE) &ssStatus,             /* address of structure */
                            sizeof(SERVICE_STATUS_PROCESS), /* size of structure */
                            &dwBytesNeeded))                /* if buffer too small */
  {
      LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());
    
    goto failed;
  }

  /* Save the tick count and initial checkpoint */
  dwStartTickCount = GetTickCount();
  dwOldCheckPoint  = ssStatus.dwCheckPoint;

  while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
  { 
    /* Do not wait longer than the wait hint. A good interval is
     * one-tenth the wait hint, but no less than 1 second and no
     * more than 10 seconds */

    dwWaitTime = ssStatus.dwWaitHint / 10;

    if(dwWaitTime < 1000)         dwWaitTime = 1000;
    else if (dwWaitTime > 10000)  dwWaitTime = 10000;

    Sleep(dwWaitTime);

    /* Check the status again */
    if (!QueryServiceStatusEx( schService,                     /* handle to service */
                               SC_STATUS_PROCESS_INFO,         /* info level */
                               (LPBYTE) &ssStatus,             /* address of structure */
                               sizeof(SERVICE_STATUS_PROCESS), /* size of structure */
                               &dwBytesNeeded ) )              /* if buffer too small */
    {
        LogAgent_Write("QueryServiceStatusEx failed (%d)\n\n", (int) GetLastError());

      break;
    }

    if(ssStatus.dwCheckPoint > dwOldCheckPoint)
    {
      /* Continue to wait and check */
      dwStartTickCount = GetTickCount();
      dwOldCheckPoint  = ssStatus.dwCheckPoint;
    }
    else
    {
      if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
      {
        /* No progress made within the wait hint */
        break;
      }
    }
  }

  /* Determine whether the service is running */
  if(ssStatus.dwCurrentState != SERVICE_RUNNING)
  {
      LogAgent_Write("Service not started ...\n\n");
      LogAgent_Write("  Current State: %d\n", (int) ssStatus.dwCurrentState);
      LogAgent_Write("  Exit Code:     %d\n", (int) ssStatus.dwWin32ExitCode);
      LogAgent_Write("  Check Point:   %d\n", (int) ssStatus.dwCheckPoint);
      LogAgent_Write("  Wait Hint:     %d\n", (int) ssStatus.dwWaitHint);

    goto failed;
  }

  CloseServiceHandle(schService);
  CloseServiceHandle(schSCManager);
  return 0;

  failed:
          if(schService)   CloseServiceHandle(schService);
          if(schSCManager) CloseServiceHandle(schSCManager);
          return 1;

} /* END: ServiceStart() */