#ifndef __WEBSERVICE_HPP__
#define __WEBSERVICE_HPP__

#include "SocketAgent.hpp"

#ifdef WIN32
    #ifdef iON
        #define WEBPAGE_PATH "iONetworks/EZ_SSD_SOS/wwwroot/dist/"
    #else
        #define WEBPAGE_PATH "Innodisk/iCAP_ClientService/wwwroot/dist/"
    #endif
#else
#define WEBPAGE_PATH "/var/iCAP_Client/wwwroot/dist/"
#endif

static char Current_User[100]={0};
typedef struct webheader
{
    char Username[1024];
    char Password[1024];
    char Token[1024];
}HEADER; 


void WEB_GETRequest(SOCKET_DATA* sock_obj);
void WEB_POSTRequest(SOCKET_DATA* sock_obj);
void WEB_PUTRequest(SOCKET_DATA* sock_obj);
#endif
