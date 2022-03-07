#ifndef __LIB_ICAPCLIENT_H__
#define __LIB_ICAPCLIENT_H__


typedef double (*GetValueFunc)(void);
typedef int (*SendRemoteFunc)(void*);

/**
 * Connect to iCAP client service
 * @return @c 0 if connection successful.
 * @return @c 1 if connection fail
 */
int iCAP_Connect();

/**
 * Disconnect to iCAP client service
 * @return @c 0 if connection successful.
 * @return @c 1 if connection fail
 */
int iCAP_Disconnect();

/**
 * Get iCAP client service status
 * @return @c 0 if iCAP client service is working in normal status.
 * @return @c 1 if iCAP client service is not working, connection to iCAP broker fail.
 * @return @c 2 if iCAP client service is not working, iCAP core service was not response.
 * @return @c 3 if iCAP client service is not working, number of devices already out of limit.
 * @return @c 4 if iCAP client service is not running.
 */
int iCAP_GetClientStatus();

/**
 * Get the iCAP client service last error message
 * @return @c The last error message.
 */
char* iCAP_GetLastErrorMsg();

/**
 * Set the iCAP client service upload dynamic raw data interval in ms
 * @param[in] ms The upload interval in million second.
 * @return @c 0 if setting successful.
 * @return @c 1 if setting fail
 */
int iCAP_SetResponseInterval(unsigned int ms);

/**
 * Add external sensor to iCAP client service.
 * @param[in] name The external sensor name.
 * @param[in] unit The external sensor data unit.
 * @param[in] type The data type of external sensor data. @c 0 numberical @c 1 switch
 * @param[] func The function pointer which returns external sensor data
 * @return @c 0 if connection successful.
 * @return @c 1 if connection fail
 */
int iCAP_AddExternalSensor(char* name, char* unit, unsigned char type, GetValueFunc func);
int iCAP_AddStaticInfo(char* name,char* info);
int iCAP_RemoveExternalSensor(char* name);
int iCAP_AddRemoteDevice(char* name, char* unit, unsigned char type, SendRemoteFunc func);
int iCAP_RemoveRemoteDevice(char* name);
int iCAP_EventTrigger();
int iCAP_SendLogout();

#endif
