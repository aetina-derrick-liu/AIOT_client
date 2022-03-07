#ifndef __DBINTERFACE_H__
#define __DBINTERFACE_H__

#ifdef WIN32
#define DBName "/Users/All Users/Innodisk/iCAP_ClientService/iCAP_ClientService.db"
#else
#define DBName "/var/iCAP_Client/iCAP_ClientService.db"
#endif

int ReadStatus();

#endif