#ifndef __ICAPAPI_H__
#define __ICAPAPI_H__


typedef struct
{
  const char  *ipAddr;
  int       itvl;   /* upload interval */
  double       lo;     /* longitude */
  double       la;     /* latitude */

} API_INFO;

#define MAX_SETTING_STR_SIZE 1024

#ifdef WIN32
#define SETTING_FILE_PATH "/Users/All Users/Innodisk/iCAP_ClientService/ServiceSetting.json"
#else
#define SETTING_FILE_PATH "/var/iCAP_Client/ServiceSetting.json"
#endif

/*----------------------------------------------------------------*/
int iCAPReCnnt (API_INFO info);


#endif