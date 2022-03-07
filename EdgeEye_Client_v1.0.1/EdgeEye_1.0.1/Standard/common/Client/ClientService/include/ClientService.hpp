#ifndef __CLIENTSERVICE_HPP__
#define __CLIENTSERVICE_HPP__

#define MAX_SETTING_STR_SIZE 1024
#define MAX_DEVICENAME_SIZE 256
#define MAX_PWD_SIZE 256
#define MAX_SERVICE_STR_SIZE 1024


int ClientService_Initialize();
int ClientService_Start();
void ClientService_Stop();
void ClientService_Restart();

#endif
