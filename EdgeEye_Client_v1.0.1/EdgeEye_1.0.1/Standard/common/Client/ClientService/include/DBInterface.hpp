#ifndef __DBINTERFACE_HPP__
#define __DBINTERFACE_HPP__

#include "ThresholdChecker.hpp"
#include "LifespanCalculator.hpp"
#include "main.hpp"
#include "WebService.hpp"

#ifdef WIN32
	#ifdef iON
		#define DBPath "iONetworks/EZ_SSD_SOS/"
		#define DBName DBPath "EZ_SSD_SOS.db"
	#else
		#define DBPath "Innodisk/iCAP_ClientService/"
		#define DBName DBPath "iCAP_ClientService.db"
	#endif
#else
#define DBPath "/var/iCAP_Client/"
#define DBName DBPath "iCAP_ClientService.db"
#endif



typedef struct{
	char* Alias;
	char* ServerIp;
	unsigned int UploadInterval;
	unsigned int DataLimitCount;
	unsigned int ResendInterval;
}SETTING_DEVICE;

typedef struct{
	int BranchId;
	double Longitude;
	double Latitude;
}SETTING_LOCATION;

#ifdef iON 
typedef struct{
	char* Alias;
	char* ServerIp;
	unsigned int UploadInterval;
	char* Account;
	char* Pwd;
	char* EZPROIP;
	unsigned int Port;
}SETTING_iON_DEVICE;
#endif


int CreateDB();
int InsertDataGroup(char* Name);
int InsertDataDefine(char* Name, char* Location, int Numberical, int GroupId);
int InsertLifespanData(char* SN, LIFESPAN_DATA* stordata, int Lifespan);
int InsertRawData(float CPULoading, float MEMLoading);
int InsertStorageData(char* SN, float TotalCapacity, float InitHealth, int PECycle);
int InsertStorageRawData(char* SN, int RawId, double Health, int Temperature, int AvgEraseCount);
int InsertThreshold(int DataId, char enable, int Func, float value);
int InsertExtData(int DataId, char* Unit);
int InsertExtDataRaw(int RawDataId,char* name,double Value);


double GetDeviceLongitude(int index);
double GetDeviceLatitude(int index);
int GetDeviceUploadInterval();
int CheckedLog(int DataId);
int CheckEventLogCount();
int CheckRawDataCount();
char* GetDashobardData();
char* GetDeviceSetting();
char* GetDeviceLocation();
char* GetDeviceRawData(int RawDataCount);
char* GetLogData(int RawDataCount);
int GetNumberOfDeviceLocation();
char* GetServiceIPAddress();
int GetStatus();
float GetLatestExtRaw(int ExtDataid);
char* GetStorageDataLastModifyDate(char* SN);
int GetStorageLifespan(char* SN);
float GetStorageInitHealth(char *SN);
int GetExtDataid(char* Name);
LIFESPAN_DATA* GetStorageParameter(char* SN);
THRESHOLD_SETTING* GetThreshold(int DataId);
char* GetThresholdSetting();
char* GetUnreadLog();
char* GetUnreadLogBaner();
int GetUnreadLogCount();
char* Gettoken();
char* GetExtUnit(int dataid);
char* Login();

int SetDeviceLocation(SETTING_LOCATION* loc_obj);
int SetDeviceName(char* DevName);
int SetDeviceParameter(SETTING_DEVICE* setting_obj);
int UpdateThreshold(int DataId, char enable, int Func, float value);
int WriteEventLog(char* msg);
int WriteStatus(int status);

int CheckUser(HEADER *header);
int CheckPermission();

struct json_object* GetDeviceInfo();

#ifdef iON
int Set_iON_DeviceParameter(SETTING_iON_DEVICE *setting_obj);
SETTING_iON_DEVICE* Get_iON_EZPROParameters();
#endif


#endif
