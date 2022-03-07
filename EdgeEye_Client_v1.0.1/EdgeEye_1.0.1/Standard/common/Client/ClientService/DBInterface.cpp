#include <string.h>
#include <iostream>
#include <unordered_set>
#include <pthread.h>
#include "sqlite3.h"
#include "Base64.hpp"
#include "WebService.hpp"
#include "DBInterface.hpp"
#include "DataBuilder.hpp"
#include "TimeEpoch.hpp"
#include "TokenGenerator.hpp"
#include "DeviceAgent.hpp"
#ifndef WIN32
extern "C"
{
	#include "CMDParser.h"
	#include "OSInfo.h"
}
#endif

extern unordered_set <string> StorList;
char header_username[1024] = {0};
pthread_mutex_t db_mutex;

#ifdef iON
#define DEVICE_SCHEMA "CREATE TABLE IF NOT EXISTS `Device` ("                     \
											"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"          \
											"`Name` VARCHAR(255) NOT NULL,"                             \
											"`Alias` VARCHAR(255),"                                     \
											"`ServerIP` VARCHAR(255) NOT NULL DEFAULT '0.0.0.0',"       \
											"`ServiceStatus` INTEGER NOT NULL DEFAULT 4,"               \
											"`UploadInterval` INTEGER NOT NULL DEFAULT 60,"             \
											"`DataLimitCount` INTEGER NOT NULL DEFAULT 10080,"          \
											"`EventResendInterval` INTEGER NOT NULL DEFAULT 600,"		\
											"`Account` VARCHAR(255),"								\
											"`Pwd` VARCHAR(255),"              \
											"`EZPROIP` VARCHAR(255) NOT NULL DEFAULT '127.0.0.1',"              \
											"`Port` INTEGER NOT NULL DEFAULT 7001,"              \
											"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
											"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"
#else
#define DEVICE_SCHEMA "CREATE TABLE IF NOT EXISTS `Device` ("                     \
											"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"          \
											"`Name` VARCHAR(255) NOT NULL,"                             \
											"`Alias` VARCHAR(255),"                                     \
											"`ServerIP` VARCHAR(255) NOT NULL DEFAULT '0.0.0.0',"       \
											"`ServiceStatus` INTEGER NOT NULL DEFAULT 4,"               \
											"`UploadInterval` INTEGER NOT NULL DEFAULT 60,"             \
											"`DataLimitCount` INTEGER NOT NULL DEFAULT 10080,"          \
											"`EventResendInterval` INTEGER NOT NULL DEFAULT 600,"				\
											"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
											"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"
#endif
#define DATAGROUP_SCHEMA "CREATE TABLE IF NOT EXISTS `DataGroup` ( "                \
												 "`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"         \
												 "`Name` VARCHAR(255) NOT NULL,"                            \
												 "`EnableUpload` BIT NOT NULL DEFAULT 1, "                  \
												 "`CreateDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP," \
												 "`LastModifiedDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP);"

#define LOCATION_SCHEMA "CREATE TABLE IF NOT EXISTS `Location` ("                         \
												"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"                \
												"`DeviceId` INTEGER NOT NULL,"                                    \
												"`BranchId` INTEGER NOT NULL,"                                    \
												"`Longitude` FLOAT NOT NULL DEFAULT 121.634977,"                    \
												"`Latitude` FLOAT NOT NULL DEFAULT 25.059067,"                     \
												"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"       \
												"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
												"FOREIGN KEY(`DeviceId`) REFERENCES `Device`(`Id`));"

#define STORAGE_SCHEMA "CREATE TABLE IF NOT EXISTS `Storage` ("                          \
											 "`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"                \
											 "`SN` VARCHAR(255) NOT NULL,"                                     \
											 "`DeviceId` INTEGER NOT NULL,"                                    \
											 "`TotalCapacity` FLOAT NOT NULL,"                                 \
											 "`CurrentCapacity` FLOAT NOT NULL,"                               \
											 "`InitialHealth` FLOAT NOT NULL,"                                 \
											 "`PECycle` INTEGER NOT NULL,"                                     \
											 "`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"       \
											 "`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
											 "FOREIGN KEY(`DeviceId`) REFERENCES `Device`(`Id`));"

#define DATA_SCHEMA "CREATE TABLE IF NOT EXISTS `Data` ("                             \
										"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"                \
										"`GroupId` INTEGER NOT NULL,"                                     \
										"`Location` VARCHAR(255),"                                        \
										"`Name` VARCHAR(255) NOT NULL,"                                   \
										"`Numberical` BIT NOT NULL DEFAULT 0,"                            \
										"`EnableUpload` BIT NOT NULL DEFAULT 1,"                          \
										"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"       \
										"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
										"FOREIGN KEY(`GroupId`) REFERENCES `DataGroup`(`Id`));"

#define EXTDATA_SCHEMA "CREATE TABLE IF NOT EXISTS `ExtData` ("                          \
											 "`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"                \
											 "`DataId` INTEGER NOT NULL,"                                      \
											 "`Unit` VARCHAR(255),"                                            \
											 "`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"       \
											 "`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
											 "FOREIGN KEY(`DataId`) REFERENCES `Data`(`Id`));"

#define RAWDATA_SCHEMA "CREATE TABLE IF NOT EXISTS `RawData` ("           \
											 "`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," \
											 "`CPULoading` FLOAT NOT NULL,"                     \
											 "`MEMLoading` FLOAT NOT NULL,"                     \
											 "`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"

#define STORAGERAW_SCHEMA "CREATE TABLE IF NOT EXISTS `StorageRaw` ("            \
													"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"     \
													"`RawDataId` INTEGER NOT NULL,"                        \
													"`StorageId` INTEGER NOT NULL,"                        \
													"`Health` FLOAT NOT NULL,"                             \
													"`Temperature` INTEGER NOT NULL,"                      \
													"`AvgEraseCount` INTEGER NOT NULL,"                    \
													"FOREIGN KEY(`RawDataId`) REFERENCES `RawData`(`Id`)," \
													"FOREIGN KEY(`StorageId`) REFERENCES `Storage`(`Id`));"

#define EXTDATARAW_SCHEMA "CREATE TABLE IF NOT EXISTS `ExtDataRaw` ("            \
													"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"     \
													"`RawDataId` INTEGER NOT NULL,"                        \
													"`ExtDataId` INTEGER NOT NULL,"                        \
													"`Value` FLOAT NOT NULL,"                              \
													"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"\
													"FOREIGN KEY(`RawDataId`) REFERENCES `RawData`(`Id`)," \
													"FOREIGN KEY(`ExtDataId`) REFERENCES `ExtData`(`Id`));" 

#define THRESHOLD_SCHEMA "CREATE TABLE IF NOT EXISTS `Threshold` ( "                      \
												 "`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"               \
												 "`DataId` INTEGER NOT NULL,"                                     \
												 "`Enable` BIT NOT NULL DEFAULT 1,"                               \
												 "`Func` INTEGER NOT NULL DEFAULT 0, "                            \
												 "`Value` FLOAT NOT NULL DEFAULT 0, "                             \
												 "`CreateDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP,"       \
												 "`LastModifiedDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP," \
												 "FOREIGN KEY(`DataId`) REFERENCES `Data`(`Id`));"

#define EVENTLOG_SCHEMA "CREATE TABLE IF NOT EXISTS `EventLog` ( "                        \
												"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"                \
												"`Checked` BIT NOT NULL DEFAULT 0,"                               \
												"`Message` VARCHAR(1024) NOT NULL,"                               \
												"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP," \
												"`CreateDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP);"

#define LIFESPAN_SCHEMA "CREATE TABLE IF NOT EXISTS `Lifespan` ( "                 \
												"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"         \
												"`StorageId` INTEGER NOT NULL,"                            \
												"`Lifespan` INTEGER NOT NULL DEFAULT 0,"                   \
												"`CreateDate` INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP," \
												"FOREIGN KEY(`StorageId`) REFERENCES `Storage`(`Id`));"
#define DEVICE_INFO_VIEW_SCHEMA "CREATE VIEW IF NOT EXISTS device_info_view AS "              \
																"SELECT Device.ServerIP, Device.ServiceStatus, Device.Name, " \
																"Device.Alias, RawData.CPULoading, RawData.MEMLoading, "      \
																"Location.Longitude, Location.Latitude "                      \
																"FROM Device "                                                \
																"INNER JOIN Location ON Location.DeviceId = 1 "               \
																"INNER JOIN RawData ORDER BY RawData.CreateDate DESC LIMIT 1;"
#define STORAGE_INFO_VIEW_SCHEMA "CREATE VIEW IF NOT EXISTS storage_info_view AS "                            \
																 "SELECT Storage.Id, Storage.SN, Storage.PECycle, StorageRaw.Health, "        \
																 "StorageRaw.AvgEraseCount, CAST(strftime('%s', RawData.CreateDate) AS INT) " \
																 "FROM StorageRaw "                                                           \
																 "INNER JOIN Storage on Storage.Id = StorageRaw.StorageId "                   \
																 "INNER JOIN RawData on RawData.Id = StorageRaw.RawDataId "                   \
																 "GROUP BY Storage.Id "                                                       \
																 "ORDER BY StorageRaw.StorageId;"
#define VERSION_SCHEMA "CREATE TABLE IF NOT EXISTS `Version` ("            \
													"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"     \
													"`Item` VARCHAR(255),"                        \
													"`Version` VARCHAR(255),"								\
													"`SetDate` VARCHAR(255)," \
													"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"\
													"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"
#define USER_SCHEMA "CREATE TABLE IF NOT EXISTS `User` ("            \
													"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"     \
													"`RightFlag` INTEGER NOT NULL,"                   \
													"`DeleteFlag` BIT NOT NULL DEFAULT 0,"                   \
													"`EmployeeNum` INTEGER,"                   \
													"`LoginName` VARCHAR(255) NOT NULL,"       \
													"`Password` VARCHAR(255) NOT NULL,"       \
													"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"\
													"`LastModifiedDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"\
													"FOREIGN KEY(`EmployeeNum`) REFERENCES `ExtData`(`Id`));" 

#define AUTHENTICATION_SCHEMA "CREATE TABLE IF NOT EXISTS `Authentication` ("            \
													"`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"     \
													"`RightFlag` INTEGER NOT NULL,"                   \
													"`LoginName` VARCHAR(255) NOT NULL,"                   \
													"`Token` VARCHAR(255) NOT NULL,"                   \
													"`CreateDate` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);"

const char *SettingHeader[6] = {"DeviceName", "Alias", "ServerIP", "UploadInterval", "DataLimitCount", "EventResendInterval"};
const char *DataLogHeader[8] = {"CreateDate", "CPULoading", "MEMLoading", "StorageId", "Health", "Temperature", "AvgEraseCount", "Lifespan"};


int CreateDB()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;

	pthread_mutex_lock(&db_mutex);

	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{

		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sqlite3_exec(db, DEVICE_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, DATAGROUP_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, LOCATION_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, STORAGE_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, DATA_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, EXTDATA_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, RAWDATA_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, STORAGERAW_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, EXTDATARAW_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, THRESHOLD_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, EVENTLOG_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, LIFESPAN_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, DEVICE_INFO_VIEW_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, STORAGE_INFO_VIEW_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, VERSION_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, USER_SCHEMA, 0, 0, &errMsg);
	sqlite3_exec(db, AUTHENTICATION_SCHEMA, 0, 0, &errMsg);

	sqlite3_get_table(db, "SELECT ServiceStatus FROM `Device` WHERE Id = 1;", &result, &rows, &cols, &errMsg);

	if (rows < 1)
	{
		sqlite3_exec(db, "INSERT INTO `Device` (Name) VALUES ('UNDEFINE')", 0, 0, &errMsg);
		sqlite3_exec(db, "INSERT INTO `Location` (DeviceId, BranchId) VALUES (1, 1)", 0, 0, &errMsg);
		sqlite3_exec(db, "INSERT INTO RawData(CPULoading, MEMLoading, CreateDate)VALUES(0, 0, CURRENT_TIMESTAMP)",
								 0, 0, &errMsg);
		sqlite3_exec(db, "INSERT INTO `Version` (Item,Version,SetDate) VALUES ('iCAP','1.2.6.0','2019-01-14');", 0, 0, &errMsg);
		sqlite3_exec(db, "INSERT INTO `User` (RightFlag,LoginName,Password) VALUES (3,'root','cm9vdA==');", 0, 0, &errMsg);
	}

	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);
	return 0;
}

int InsertDataGroup(char *Name)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char update_str[300]= {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM DataGroup WHERE Name = '%s';", Name);

	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		memset(update_str, 0, 300);
		sprintf(update_str, "INSERT INTO DataGroup(Name, EnableUpload, CreateDate, LastModifiedDate)"
												"VALUES('%s', 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
						Name);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);

		sprintf(update_str, "SELECT Id FROM DataGroup WHERE Name = '%s';", Name);

		sqlite3_free_table(result);

		sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

		if (rows > 0)
		{
			ret = atoi(*(result + 1));
		}
		else
		{
			ret = -2;
		}
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int InsertDataDefine(char *Name, char *Location, int Numberical, int GroupId)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char update_str[300];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM Data WHERE Name = '%s';", Name);

	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		memset(update_str, 0, 300);
		sprintf(update_str, "INSERT INTO Data(Name, GroupId, Location, Numberical, EnableUpload, CreateDate, LastModifiedDate)"
												"VALUES('%s', %d, '%s', %d, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
						Name,
						GroupId,
						Location,
						Numberical);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);

		memset(update_str, 0, 300);
		sprintf(update_str, "SELECT Id FROM Data WHERE Name = '%s';", Name);

		sqlite3_free_table(result);

		sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

		if (rows > 0)
		{
			ret = atoi(*(result + 1));
		}
		else
		{
			ret = -2;
		}
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int InsertLifespanData(char *SN, LIFESPAN_DATA *stordata, int Lifespan)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char update_str[1024]={0};
	int StorageId = 0;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str,
					"UPDATE Storage SET CurrentCapacity = '%f', LastModifiedDate = CURRENT_TIMESTAMP WHERE SN = '%s';",
					stordata->CurrentCapacity,
					SN);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	memset(update_str,0,1024);

	sprintf(update_str,
					"SELECT Id FROM Storage WHERE SN = '%s'",
					SN);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows < 0)
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -3;
	}

	if(*(result + 1))
	{
		StorageId = strtol(*(result + 1),NULL,0);
		memset(update_str,0,1024);

		sprintf(update_str,
					"INSERT INTO Lifespan(StorageId, Lifespan) VALUES(%d, %d);",
					StorageId,
					Lifespan);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}

int InsertRawData(float CPULoading, float MEMLoading)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret = 0;
	char update_str[300]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	if(CPULoading!=0)
	{
		sprintf(update_str, "INSERT INTO RawData(CPULoading, MEMLoading, CreateDate)"
											"VALUES(%f, %f, CURRENT_TIMESTAMP)",
					CPULoading,
					MEMLoading);
	}

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_get_table(db, "SELECT seq FROM sqlite_sequence WHERE name = 'RawData';", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		ret = -2;
	}

	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int InsertStorageData(char *SN, float TotalCapacity, float InitHealth, int PECycle)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char update_str[300];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM Storage WHERE SN = '%s';", SN);

	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		memset(update_str, 0, 300);
		sprintf(update_str, "INSERT INTO Storage(SN, DeviceId, TotalCapacity, CurrentCapacity, InitialHealth, PECycle)"
												"VALUES('%s', 1, %f, %f, %f, %d)",
						SN,
						TotalCapacity,
						TotalCapacity * (InitHealth / 100.0),
						InitHealth,
						PECycle);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);

		sprintf(update_str, "SELECT Id FROM Storage WHERE SN = '%s';", SN);

		sqlite3_free_table(result);

		sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

		if (rows > 0)
		{
			ret = atoi(*(result + 1));
		}
		else
		{
			ret = -2;
		}
	}

	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int InsertStorageRawData(char *SN, int RawId, double Health, int Temperature, int AvgEraseCount)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols, rows1, cols1;
	char **result,**result1;
	int ret = 0;
	char update_str[300] = {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM Storage WHERE SN = '%s';", SN);
	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));

		memset(update_str, 0, 300);
		sprintf(update_str, "INSERT INTO StorageRaw(RawDataId, StorageId, Health, Temperature, AvgEraseCount)"
												"VALUES(%d, %d, %f, %d, %d)",
						RawId,
						ret,
						Health,
						Temperature,
						AvgEraseCount);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);
		memset(update_str, 0, 300);
		sprintf(update_str, "SELECT seq FROM sqlite_sequence WHERE name = 'RawData';");

		sqlite3_get_table(db, update_str, &result1, &rows1, &cols1, &errMsg);
		if (rows > 0)
		{
			ret = atoi(*(result1 + 1));
		}
		else
		{
			ret = -2;
		}
		sqlite3_free_table(result1);
	}
	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int InsertThreshold(int DataId, char enable, int Func, float value)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char update_str[500];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM Threshold WHERE DataId = %d AND Value = %f;", DataId , value);

	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		memset(update_str, 0, 500);
		sprintf(update_str, "INSERT INTO Threshold(DataId, Enable, Func, Value, CreateDate, LastModifiedDate)"
												"VALUES(%d, %d, %d, %f, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
						DataId,
						enable,
						Func,
						value);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);

		memset(update_str, 0, 500);
		sprintf(update_str, "SELECT Id FROM Threshold WHERE DataId = %d;", DataId);

		sqlite3_free_table(result);

		sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

		if (rows > 0)
		{
			ret = atoi(*(result + 1));
		}
		else
		{
			ret = -2;
		}
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}
int InsertExtData(int DataId, char* Unit)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret = 0;
	char query_str[300];
	char update_str[300];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}
	sprintf(query_str,
					"SELECT DataId "
					"FROM ExtData "
					"WHERE ExtData.DataId = '%d' ",
					DataId);
	sqlite3_get_table(db, query_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}
	else
	{
		sprintf(update_str, "INSERT INTO ExtData(DataId, Unit , CreateDate, LastModifiedDate)"
											"VALUES(%d,'%s', CURRENT_TIMESTAMP,CURRENT_TIMESTAMP)",
					DataId,
					Unit);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);
	}
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}
int InsertExtDataRaw(int RawDataId,char* name,double Value)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret = 0;
	char query_str[300] = {0};
	char update_str[300] = {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}
	sprintf(query_str,
					"SELECT Id "
					"FROM Data "
					"WHERE Data.Name = '%s' ",
					name);
	sqlite3_get_table(db, query_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atof(*(result + 1));
	}

	sqlite3_free_table(result);
	sprintf(update_str, "INSERT INTO ExtDataRaw(RawDataId, ExtDataId , Value)"
											"VALUES(%d,%d,%f)",
					RawDataId,
					ret,
					Value);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return 0;
}

double GetDeviceLongitude(int index)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	double ret;
	char queryStr[255];

	sprintf(queryStr, "SELECT Longitude FROM Location WHERE Id = %d;", index);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, queryStr, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atof(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

double GetDeviceLatitude(int index)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	double ret;
	char queryStr[255];

	sprintf(queryStr, "SELECT Latitude FROM Location WHERE Id = %d;", index);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, queryStr, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atof(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int GetDeviceUploadInterval()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT UploadInterval FROM Device WHERE Id = 1;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}

	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int CheckedLog(int DataId)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[1024];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);

		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `EventLog` SET Checked = 1, LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = %d;", DataId);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}

int CheckEventLogCount()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret = 0;
	char update_str[300];
	int DataLimit, DataCount, DeleteId;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sqlite3_get_table(db, "SELECT Count(EventLog.Id), Device.DataLimitCount FROM EventLog INNER JOIN Device on Device.Id = 1;", &result, &rows, &cols, &errMsg);

	if (!(rows > 0))
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -3;
	}

	DataCount = atoi(*(result + 1));
	DataLimit = atoi(*(result + 2));

	if (DataCount < DataLimit)
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sqlite3_free_table(result);

	sqlite3_get_table(db, "SELECT Id FROM EventLog ORDER BY CreateDate LIMIT 1;", &result, &rows, &cols, &errMsg);
	if (!(rows > 0))
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -4;
	}

	DeleteId = atoi(*(result + 1));

	memset(update_str, 0, 300);
	sprintf(update_str, "DELETE FROM EventLog WHERE Id = %d;", DeleteId);
	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int CheckRawDataCount()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret = 0;
	char update_str[300];
	int DataLimit, DataCount, DeleteId;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sqlite3_get_table(db, "SELECT Count(RawData.Id), Device.DataLimitCount FROM RawData INNER JOIN Device on Device.Id = 1;", &result, &rows, &cols, &errMsg);

	if (!(rows > 0))
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -3;
	}

	DataCount = atoi(*(result + 1));
	DataLimit = atoi(*(result + 2));

	if (DataCount < DataLimit)
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sqlite3_free_table(result);

	sqlite3_get_table(db, "SELECT Id FROM RawData ORDER BY CreateDate LIMIT 1;", &result, &rows, &cols, &errMsg);
	if (!(rows > 0))
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -4;
	}

	DeleteId = atoi(*(result + 1));

	memset(update_str, 0, 300);
	sprintf(update_str, "DELETE FROM StorageRaw WHERE RawDataId = %d;", DeleteId);
	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	memset(update_str, 0, 300);
	sprintf(update_str, "DELETE FROM RawData WHERE Id = %d;", DeleteId);
	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetDashobardData()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols, rows2, cols2, first_stor_index = 0;
	char **result, **result2;
	struct json_object *ret_obj = json_object_new_object();
	struct json_object *ret_obj_arr = NULL;
	struct json_object *ret_obj2 = NULL;
	char *ret = (char *)malloc(sizeof(char) * 65536);
	memset(ret,0,65536);
	char queryStr[1024] = {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT * FROM device_info_view;", &result, &rows, &cols, &errMsg);

	ret_obj_arr = json_object_new_array();
	ret_obj2 = json_object_new_object();

	if (rows > 0)
	{	
		json_object_object_add(ret_obj, "ServerIP", json_object_new_string(*(result + cols)));
		json_object_object_add(ret_obj, "ServiceStatus", json_object_new_int(atoi(*(result + cols + 1))));
		json_object_object_add(ret_obj, "Name", json_object_new_string(*(result + cols + 2)));

		if (*(result + cols + 3))
		{
			json_object_object_add(ret_obj, "Alias", json_object_new_string(*(result + cols + 3)));
		}
		else
		{
			json_object_object_add(ret_obj, "Alias", json_object_new_string(""));
		}

		json_object_object_add(ret_obj, "CPULoading", json_object_new_int(atoi(*(result + cols + 4))));
		json_object_object_add(ret_obj, "MEMLoading", json_object_new_double(strtod(*(result + cols + 5),NULL)));
		json_object_object_add(ret_obj2, "Longitude", json_object_new_double(atof(*(result + cols + 6))));
		json_object_object_add(ret_obj2, "Latitude", json_object_new_double(atof(*(result + cols + 7))));
		json_object_array_add(ret_obj_arr, ret_obj2);
		json_object_object_add(ret_obj, "Location", ret_obj_arr);
	}
	else
	{
		json_object_object_add(ret_obj, "ServerIP", json_object_new_string("0.0.0.0"));
		json_object_object_add(ret_obj, "ServiceStatus", json_object_new_int(2));
		json_object_object_add(ret_obj, "Name", json_object_new_string(""));
		json_object_object_add(ret_obj, "Alias", json_object_new_string(""));

		json_object_object_add(ret_obj, "CPULoading", json_object_new_string("N/A"));
		json_object_object_add(ret_obj, "MEMLoading", json_object_new_string("N/A"));
		json_object_object_add(ret_obj2, "Longitude", json_object_new_double(100.0));
		json_object_object_add(ret_obj2, "Latitude", json_object_new_double(100.0));
		json_object_array_add(ret_obj_arr, ret_obj2);
		json_object_object_add(ret_obj, "Location", ret_obj_arr);
	}

	sqlite3_free_table(result);

	sqlite3_get_table(db, "SELECT * FROM storage_info_view;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		struct json_object *ret_obj_arr2 = json_object_new_array();
		for (int i = 6; i < (rows + 1) * cols; i += 6)
		{
			struct json_object *ret_obj3 = json_object_new_object();
			json_object_object_add(ret_obj3, "Index", json_object_new_int(atoi(*(result + i))));
			if (first_stor_index == 0)
			{
				first_stor_index = atoi(*(result + i));
			}
			if(StorList.find(string(*(result + i + 1))) == StorList.end())	continue;
			json_object_object_add(ret_obj3, "SN", json_object_new_string(*(result + i + 1)));
			json_object_object_add(ret_obj3, "PECycle", json_object_new_int(atoi(*(result + i + 2))));
			json_object_object_add(ret_obj3, "Health", json_object_new_double(atof(*(result + i + 3))));
			json_object_object_add(ret_obj3, "AvgEraseCount", json_object_new_int(atoi(*(result + i + 4))));
			json_object_object_add(ret_obj3, "LastUploadTime", json_object_new_int(atoi(*(result + i + 5))));

			//Temperature
			memset(queryStr, 0, 1024);

			sprintf(queryStr,
							"SELECT * FROM ("
							"SELECT Temperature, CAST(strftime('%%s', RawData.CreateDate) AS INT) AS Time "
							"FROM StorageRaw "
							"JOIN Storage ON Storage.SN = '%s' "
							"JOIN RawData ON RawData.Id = RawDataId "
							"WHERE StorageRaw.StorageId = Storage.Id "
							"ORDER BY RawData.CreateDate DESC "
							"LIMIT 10) AS T1 "
							"ORDER BY Time",
							*(result + i + 1));

			sqlite3_get_table(
					db,
					queryStr,
					&result2,
					&rows2,
					&cols2,
					&errMsg);

			struct json_object *ret_obj_temp = json_object_new_array();

			for (int j = 2; j < (rows2 + 1) * cols2; j += 2)
			{
				json_object_array_add(ret_obj_temp, json_object_new_int(atoi(*(result2 + j))));
			}

			json_object_object_add(ret_obj3, "Temperature", ret_obj_temp);
	
			sqlite3_free_table(result2);

			//Lifespan
			memset(queryStr, 0, 1024);

			sprintf(queryStr,
							"SELECT * FROM ("
							"SELECT CAST(strftime('%%s', Lifespan.CreateDate) AS INT) AS Time, Lifespan "
							"FROM Lifespan "
							"JOIN Storage ON Storage.SN = '%s' "
							"WHERE Lifespan.StorageId = Storage.Id "
							"ORDER BY Lifespan.CreateDate DESC "
							"LIMIT 10) AS T1 "
							"ORDER BY Time",
							*(result + i + 1));

			sqlite3_get_table(
					db,
					queryStr,
					&result2,
					&rows2,
					&cols2,
					&errMsg);

			struct json_object *ret_obj4 = json_object_new_object();
			struct json_object *ret_obj_life = json_object_new_array();
			struct json_object *ret_obj_life_time = json_object_new_array();

			for (int j = 2; j < (rows2 + 1) * cols2; j += 2)
			{
				json_object_array_add(ret_obj_life_time, json_object_new_int(atoi(*(result2 + j))));
				json_object_array_add(ret_obj_life, json_object_new_int(atoi(*(result2 + j + 1))));
			}

			json_object_object_add(ret_obj4, "Time", ret_obj_life_time);
			json_object_object_add(ret_obj4, "Days", ret_obj_life);
			json_object_object_add(ret_obj3, "Lifespan", ret_obj4);

			json_object_array_add(ret_obj_arr2, ret_obj3);
			sqlite3_free_table(result2);
		}

		json_object_object_add(ret_obj, "StorList", ret_obj_arr2);
	}

	sqlite3_free_table(result);
	memset(queryStr, 0, 1024);

	sprintf(queryStr,
					"SELECT * FROM ("
					"SELECT CAST(strftime('%%s', RawData.CreateDate) AS INT) AS Time "
					"FROM StorageRaw "
					"JOIN RawData ON RawData.Id = RawDataId "
					"WHERE StorageRaw.StorageId = %d "
					"ORDER BY RawData.CreateDate DESC "
					"LIMIT 10) AS T1 "
					"ORDER BY Time;",
					first_stor_index);

	sqlite3_get_table(db, queryStr, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		struct json_object *ret_obj_time = json_object_new_array();

		for (int i = 1; i < (rows + 1) * cols; i++)
		{
			json_object_array_add(ret_obj_time, json_object_new_int(atoi(*(result + i))));
		}

		json_object_object_add(ret_obj, "RawDataTime", ret_obj_time);
	}
	else
	{
		struct json_object *ret_obj_time = json_object_new_array();

		json_object_object_add(ret_obj, "RawDataTime", ret_obj_time);
	}

	sqlite3_free_table(result);
	strcpy(ret, json_object_get_string(ret_obj));
	json_object_put(ret_obj);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

struct json_object* GetDeviceInfo()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;

	struct json_object* ret = json_object_new_object();

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT Name,Alias FROM `device_info_view`;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		json_object_object_add(ret, "Name", json_object_new_string(*(result+2)));
		if(*(result+3))
		{
			json_object_object_add(ret, "Alias", json_object_new_string(*(result+3)));
		}
		else
		{
			json_object_object_add(ret, "Alias", json_object_new_string(""));
		}
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}
char *Gettoken()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char *ret = (char *)malloc(sizeof(char) * 255);
	memset(ret,0,255);
	char query_str[1024]= {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}
	sprintf(query_str, "SELECT Token from Authentication WHERE Authentication.LoginName='%s';",header_username);
	sqlite3_get_table(db,query_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		strcpy(ret, *(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);
	return ret;

}
char* Login()
{
	char* token;
	char *ret_str = (char *)malloc(sizeof(char) * 65536);
	memset(ret_str,0,65536);
	struct json_object *ret_obj = json_object_new_object();
	token = Gettoken();
	json_object_object_add(ret_obj, "Token", json_object_new_string(token));
	strcpy(ret_str, json_object_get_string(ret_obj));
	json_object_put(ret_obj);
	free(token);
	return ret_str;
}
char *GetDeviceSetting()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_object();
	char *ret = (char *)malloc(sizeof(char) * 1024);
	memset(ret,0,1024);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db,
					"SELECT Name, Alias, ServerIP, UploadInterval, DataLimitCount, EventResendInterval FROM `Device` WHERE Id = 1;",
					&result,
					&rows,
					&cols,
					&errMsg);

	if (rows > 0)
	{
		for (int i = cols; i < cols * 2; i++)
		{
			if (*(result + i))
			{
				if (i < (cols + 3))
				{
					json_object_object_add(ret_obj, SettingHeader[i - cols], json_object_new_string(*(result + i)));
				}
				else
				{
					json_object_object_add(ret_obj, SettingHeader[i - cols], json_object_new_int(atoi(*(result + i))));
				}
			}
			else
			{
				if (i < (cols + 3))
				{
					json_object_object_add(ret_obj, SettingHeader[i - cols], json_object_new_string(""));
				}
				else
				{
					json_object_object_add(ret_obj, SettingHeader[i - cols], json_object_new_int(0));
				}
			}
		}
	}

#ifdef iON
	sqlite3_free_table(result);
	struct json_object *EZRPO_obj = json_object_new_object();
	sqlite3_get_table(db,
										"SELECT Account, Pwd, EZPROIP, Port FROM `Device` WHERE Id = 1;",
										&result,
										&rows,
										&cols,
										&errMsg);
	if (rows > 0)
	{
		if(*(result + 4))
		{
			json_object_object_add(EZRPO_obj, "Account", json_object_new_string(*(result + 4)));
		}
		else
		{
			json_object_object_add(EZRPO_obj, "Account", json_object_new_string(""));
		}
		if(*(result + 5))
		{
			json_object_object_add(EZRPO_obj, "Password", json_object_new_string(*(result + 5)));
		}
		else
		{
			json_object_object_add(EZRPO_obj, "Password", json_object_new_string(""));
		}
		json_object_object_add(EZRPO_obj, "EZPROIP", json_object_new_string(*(result + 6)));
		json_object_object_add(EZRPO_obj, "Port", json_object_new_int(atoi(*(result + 7))));
		json_object_object_add(ret_obj,"EZPRO",EZRPO_obj);
	}

#endif

	strcpy(ret, json_object_get_string(ret_obj));
	json_object_put(ret_obj);
	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetDeviceLocation()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 1024);
	memset(ret,0,1024);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db,
										"SELECT Longitude, Latitude FROM `Location` WHERE DeviceId = 1;",
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{

		for (int i = 2; i < (rows + 1) * cols; i += 2)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, "Longitude", json_object_new_double(atof(*(result + i))));
			json_object_object_add(ret_obj2, "Latitude", json_object_new_double(atof(*(result + i + 1))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetDeviceRawData(int RawDataCount)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 8192);
	memset(ret,0,8192);
	char queryStr[1024]= {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sprintf(queryStr,
					"SELECT CAST(strftime('%%s', CreateDate) AS INT),"
					" CPULoading,"
					" MEMLoading,"
					" StorageId,"
					" Health,"
					" Temperature,"
					" AvgEraseCount,"
					" Lifespan"
					" FROM `RawData`"
					" INNER JOIN StorageRaw ON StorageRaw.RawDataId = RawData.Id"
					" INNER JOIN Storage ON Storage.Id = StorageRaw.StorageId"
					" INNER JOIN Lifespan ON Lifespan.StorageId = Storage.Id"
					" ORDER BY CreateDate DESC LIMIT %d;",
					RawDataCount);

	sqlite3_get_table(db,
										queryStr,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		for (int i = 8; i < (rows + 1) * cols; i += 8)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, DataLogHeader[0], json_object_new_int(atoi(*(result + i))));
			json_object_object_add(ret_obj2, DataLogHeader[1], json_object_new_int(atoi(*(result + i + 1))));
			json_object_object_add(ret_obj2, DataLogHeader[2], json_object_new_double(atof(*(result + i + 2))));
			json_object_object_add(ret_obj2, DataLogHeader[3], json_object_new_int(atoi(*(result + i + 3))));
			json_object_object_add(ret_obj2, DataLogHeader[4], json_object_new_double(atof(*(result + i + 4))));
			json_object_object_add(ret_obj2, DataLogHeader[5], json_object_new_int(atoi(*(result + i + 5))));
			json_object_object_add(ret_obj2, DataLogHeader[6], json_object_new_int(atoi(*(result + i + 6))));
			json_object_object_add(ret_obj2, DataLogHeader[7], json_object_new_int(atoi(*(result + i + 7))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetLogData(int RawDataCount)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 65536);
	memset(ret,0,65536);
	char queryStr[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	if (RawDataCount < 0)
	{
		sprintf(queryStr,
						"SELECT Id,"
						" Message,"
						" Checked,"
						"	CAST(strftime('%%s', CreateDate) AS INT)"
						" FROM `EventLog`"
						" ORDER BY CreateDate DESC;");
	}
	else
	{
		sprintf(queryStr,
						"SELECT Id,"
						" Message,"
						" Checked,"
						"	CAST(strftime('%%s', CreateDate) AS INT)"
						" FROM `EventLog`"
						" ORDER BY CreateDate DESC LIMIT %d;",
						RawDataCount);
	}

	sqlite3_get_table(db,
										queryStr,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		for (int i = 4; i < (rows + 1) * cols; i += 4)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, "Id", json_object_new_int(atoi(*(result + i))));
			json_object_object_add(ret_obj2, "Message", json_object_new_string(*(result + i + 1)));
			json_object_object_add(ret_obj2, "Checked", json_object_new_int(atoi(*(result + i + 2))));
			json_object_object_add(ret_obj2, "Time", json_object_new_double(atof(*(result + i + 3))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int GetNumberOfDeviceLocation()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT COUNT(*) FROM Location;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetServiceIPAddress()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char *ret = (char *)malloc(sizeof(char) * 255);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT ServerIP FROM `Device` WHERE Id = 1;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		strcpy(ret, *(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int GetStatus()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db, "SELECT ServiceStatus FROM `Device` WHERE Id = 1;", &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char* GetStorageDataLastModifyDate(char *SN)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char* time;
	char update_str[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(update_str,
					"SELECT CreateDate FROM Storage WHERE SN = '%s';",
					SN);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		time=(char*)malloc(sizeof(char)*20);
		memset(time,0,20);
		strcpy(time,*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return time;
}

float GetStorageInitHealth(char *SN)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	float InitHealth;
	char update_str[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(update_str,
					"SELECT InitialHealth FROM Storage WHERE SN = '%s';",
					SN);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		InitHealth = atof(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return InitHealth;
}

int GetStorageLifespan(char *SN)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int lifespan;
	char update_str[1024]= {0};

	lifespan = -1;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(update_str,
					"SELECT Lifespan "
					"FROM Lifespan "
					"JOIN Storage ON Storage.SN = '%s' "
					"WHERE Lifespan.StorageId = Storage.Id "
					"ORDER BY Lifespan.CreateDate DESC "
					"LIMIT 1;",
					SN);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		lifespan = atoi(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return lifespan;
}
int GetExtDataid(char* Name)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int dataid;
	char update_str[1024]= {0};

	dataid = -1;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(update_str,
					"SELECT Id "
					"FROM Data "
					"WHERE Data.Name= '%s'",
					Name);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		dataid = atoi(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return dataid;
}
float GetLatestExtRaw(int ExtDataid)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	float latest_value = 0;
	char query_str[1024]= {0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(query_str,"SELECT Value FROM ExtDataRaw WHERE ExtDataId = %d ORDER BY -RawDataId LIMIT 1;",ExtDataid);

	sqlite3_get_table(db,
										query_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		latest_value = atof(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return latest_value;
}
LIFESPAN_DATA *GetStorageParameter(char *SN)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char update_str[1024]= {0};
	LIFESPAN_DATA *ret = (LIFESPAN_DATA *)malloc(sizeof(LIFESPAN_DATA));

	ret->InitHealth = 0;
	ret->TotalCapacity = 0;
	ret->CurrentCapacity = 0;
	ret->PECycle = 0;
	ret->CreateDate = 0;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sprintf(update_str,
					"SELECT InitialHealth, TotalCapacity, CurrentCapacity, PECycle, CAST(strftime('%%s', CreateDate) AS INT) FROM Storage WHERE SN = '%s';",
					SN);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		ret->InitHealth = atof(*(result + 5));
		ret->TotalCapacity = atof(*(result + 6));
		ret->CurrentCapacity = atof(*(result + 7));
		ret->PECycle = atoi(*(result + 8));
		ret->CreateDate = atoi(*(result + 9));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

THRESHOLD_SETTING *GetThreshold(int DataId)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char update_str[1024]= {0};
	THRESHOLD_SETTING *ret = (THRESHOLD_SETTING *)malloc(sizeof(THRESHOLD_SETTING));

	ret->Func = 0;
	ret->Enable = 0;
	ret->Value = 0;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sprintf(update_str, "SELECT Func, Enable, Value FROM `Threshold` WHERE DataId = %d;", DataId);

	sqlite3_get_table(db,
										update_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		ret->Func = atoi(*(result + 3));
		ret->Enable = atoi(*(result + 4));
		ret->Value = atof(*(result + 5));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}
char* GetExtUnit(int dataid)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char* ret=(char*)malloc(sizeof(char)*10);
	memset(ret,0,10);
	char query_str[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sprintf(query_str,
					"SELECT Unit "
					"FROM ExtData "
					"WHERE ExtData.DataId= '%d'",
					dataid);

	sqlite3_get_table(db,
										query_str,
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		strcpy(ret, *(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetThresholdSetting()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 65536);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db,
										"SELECT Data.Id, Name, Func, Enable, Value FROM `Threshold` INNER JOIN Data ON Data.Id = Threshold.DataId;",
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{

		for (int i = 5; i < (rows + 1) * cols; i += 5)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, "Id", json_object_new_int(atoi(*(result + i))));
			json_object_object_add(ret_obj2, "Name", json_object_new_string(*(result + i + 1)));
			json_object_object_add(ret_obj2, "Func", json_object_new_int(atoi(*(result + i + 2))));
			json_object_object_add(ret_obj2, "Enable", json_object_new_int(atoi(*(result + i + 3))));
			json_object_object_add(ret_obj2, "Value", json_object_new_double(atof(*(result + i + 4))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetUnreadLog()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 65536);
	memset(ret,0,65536);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db,
										"SELECT Id, Message, CAST(strftime('%%s', CreateDate) AS INT)"
										" FROM `EventLog` WHERE Checked = 0 ORDER BY CreateDate DESC;",
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		for (int i = 3; i < (rows + 1) * cols; i += 3)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, "Id", json_object_new_int(atoi(*(result + i))));
			json_object_object_add(ret_obj2, "Message", json_object_new_string(*(result + i + 1)));
			json_object_object_add(ret_obj2, "Time", json_object_new_double(atof(*(result + i + 2))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

char *GetUnreadLogBaner()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	struct json_object *ret_obj = json_object_new_array();
	char *ret = (char *)malloc(sizeof(char) * 65536);

	memset(ret,0,65536);
	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		json_object_put(ret_obj);
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sqlite3_get_table(db,
										"SELECT Id, Message, CAST(strftime('%s', CreateDate) AS INT)"
										" FROM `EventLog` WHERE Checked = 0 ORDER BY CreateDate DESC LIMIT 20;",
										&result, &rows,	&cols, &errMsg);

	if (rows > 0)
	{
		for (int i = 3; i < (rows + 1) * cols; i += 3)
		{
			struct json_object *ret_obj2 = json_object_new_object();
			json_object_object_add(ret_obj2, "Id", json_object_new_int(atoi(*(result + i))));
			json_object_object_add(ret_obj2, "Message", json_object_new_string(*(result + i + 1)));
			json_object_object_add(ret_obj2, "Time", json_object_new_double(atof(*(result + i + 2))));
			json_object_array_add(ret_obj, ret_obj2);
		}
		strcpy(ret, json_object_get_string(ret_obj));
	}

	json_object_put(ret_obj);
	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int GetUnreadLogCount()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols, count = 0;
	char **result;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return 0;
	}

	sqlite3_get_table(db,
										"SELECT COUNT(*) FROM EventLog WHERE Checked=0;",
										&result,
										&rows,
										&cols,
										&errMsg);

	if (rows > 0)
	{
		count = atoi(*(result + 1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return count;
}

int SetDeviceLocation(SETTING_LOCATION *loc_obj)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `Location` SET Longitude = '%f', Latitude = '%f', LastModifiedDate = CURRENT_TIMESTAMP WHERE BranchId = %d;", loc_obj->Longitude, loc_obj->Latitude, loc_obj->BranchId);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}

int CheckUpdateAlias(char* new_alias)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows = 0, cols = 0;
	char **result;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return NULL;
	}

	sqlite3_get_table(db,"SELECT ServiceStatus FROM `device_info_view`;",&result,&rows,&cols,&errMsg);

	if(rows > 0)
	{
		if(atoi(*(result+1))==0)
		{
			SendUpdatedAlias(new_alias);
		}
	}

	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return 0;
}

int SetDeviceName(char *DevName)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[100]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `Device` SET Name = '%s', LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = 1;", DevName);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return 0;
}

int SetDeviceParameter(SETTING_DEVICE *setting_obj)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[1024]={0};

	CheckUpdateAlias(setting_obj->Alias);

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `Device` SET Alias = '%s', ServerIP = '%s', UploadInterval = %d, DataLimitCount = %d, EventResendInterval = %d, LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = 1;", setting_obj->Alias, setting_obj->ServerIp, setting_obj->UploadInterval, setting_obj->DataLimitCount, setting_obj->ResendInterval);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}
#ifdef iON 
SETTING_iON_DEVICE* Get_iON_EZPROParameters()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	SETTING_iON_DEVICE* ez = (SETTING_iON_DEVICE*)malloc(sizeof(SETTING_iON_DEVICE));
	memset(ez,0,sizeof(SETTING_iON_DEVICE));

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return NULL;
	}

	sqlite3_get_table(db,
										"SELECT Account, Pwd, EZPROIP, Port FROM `Device` WHERE Id = 1;",
										&result,
										&rows,
										&cols,
										&errMsg);
	if (rows > 0)
	{
		if(*(result + 4))
		{
			ez->Account=(char*)malloc(sizeof(char)*20);
			strcpy(ez->Account,*(result + 4));
		}
		if(*(result + 5))
		{
			ez->Pwd=(char*)malloc(sizeof(char)*20);
			strcpy(ez->Pwd,*(result + 5));
		}
		ez->EZPROIP=(char*)malloc(sizeof(char)*20);
		strcpy(ez->EZPROIP,*(result + 6));
		ez->Port=atoi(*(result + 7));
	}


	sqlite3_free_table(result);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ez;

}
int Set_iON_DeviceParameter(SETTING_iON_DEVICE *setting_obj)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[1024]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `Device` SET Alias = '%s', ServerIP = '%s', UploadInterval = %d, Account = '%s', Pwd = '%s', EZPROIP = '%s' , Port = %d  ,LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = 1;"
	, setting_obj->Alias, setting_obj->ServerIp, setting_obj->UploadInterval, setting_obj->Account, setting_obj->Pwd, setting_obj->EZPROIP, setting_obj->Port);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}
#endif
int UpdateThreshold(int DataId, char enable, int Func, float value)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char update_str[500]={0};

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "SELECT Id FROM Threshold WHERE DataId = %d;", DataId);

	sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

	if (rows > 0)
	{
		ret = atoi(*(result + 1));

		memset(update_str, 0, 500);
		sprintf(update_str, "UPDATE `Threshold` SET Enable = %d, Func = %d, Value = %f, LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = %d;", enable, Func, value, ret);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);
	}
	else
	{
		sprintf(update_str, "INSERT INTO Threshold(DataId, Enable, Func, Value, CreateDate, LastModifiedDate)"
												"VALUES(%d, %d, %d, %f, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
						DataId,
						enable,
						Func,
						value);

		sqlite3_exec(db, update_str, 0, 0, &errMsg);

		memset(update_str, 0, 500);
		sprintf(update_str, "SELECT Id FROM Threshold WHERE DataId = %d;", DataId);

		sqlite3_free_table(result);

		sqlite3_get_table(db, update_str, &result, &rows, &cols, &errMsg);

		if (rows > 0)
		{
			ret = atoi(*(result + 1));
		}
		else
		{
			ret = -2;
		}
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	return ret;
}

int WriteEventLog(char *msg)
{
	sqlite3 *db;
	char *errMsg = NULL, **result, update_str[1500];
	int rows, cols, interval = 0;
	long time_diff = 0;

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sqlite3_get_table(db, "SELECT EventResendInterval FROM Device;", &result,	&rows, &cols,	&errMsg);

	if(rows > 0)
	{
		interval = strtol(*(result + 1), NULL, 0);
	}

	sprintf(update_str, "SELECT CreateDate FROM EventLog WHERE Message = '%s' ORDER BY CreateDate DESC LIMIT 1;", msg);

	sqlite3_free_table(result);

	sqlite3_get_table(db, update_str, &result,	&rows, &cols,	&errMsg);

	if(rows > 0)
	{
		time_diff = GetEpochDifferenceFromString(*(result+1));
		if(time_diff < interval)
		{
			sqlite3_free_table(result);
			sqlite3_close(db);
			pthread_mutex_unlock(&db_mutex);
			return 0;
		}
	}

	sprintf(update_str, "INSERT INTO EventLog(Message, CreateDate) VALUES('%s', CURRENT_TIMESTAMP)", msg);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_free_table(result);
	sqlite3_close(db);

	pthread_mutex_unlock(&db_mutex);

	if (errMsg != NULL)
	{
		return -2;
	}

	return 0;
}

int WriteStatus(int status)
{
	sqlite3 *db;
	char *errMsg = NULL;
	char update_str[100];

	pthread_mutex_lock(&db_mutex);


	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
	
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(update_str, "UPDATE `Device` SET ServiceStatus = %d, LastModifiedDate = CURRENT_TIMESTAMP WHERE Id = 1;", status);

	sqlite3_exec(db, update_str, 0, 0, &errMsg);

	sqlite3_close(db);


	pthread_mutex_unlock(&db_mutex);

	return status;
}

int CheckUser(HEADER *header)
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int ret;
	char query_str[300] = {0};
	char update_str[300] = {0};
	char pwd_encode[300] = {0};

	pthread_mutex_lock(&db_mutex);

	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return ret;
	}

	sprintf(query_str, "SELECT RightFlag,Password from User WHERE User.LoginName='%s' AND User.DeleteFlag = 0;",header->Username);
	sqlite3_get_table(db,query_str, &result, &rows, &cols, &errMsg);
	if (rows > 0)
	{
		base64_encode((unsigned char*)(header->Password),
				  		(unsigned char*)pwd_encode,
				  		strlen(header->Password),
				  		0);
		if(strcmp(*(result + cols + 1),pwd_encode)==0)
		{
			int RightFlag=atoi(*(result + cols));
			sqlite3_free_table(result);
			sprintf(query_str, "SELECT * from Authentication WHERE Authentication.LoginName='%s';",header->Username);
			sqlite3_get_table(db,query_str, &result, &rows, &cols, &errMsg);
			if(rows > 0)
			{
			}
			else
			{
				char* random_str=(char*)malloc(sizeof(char)*100);
				char* token=(char*)malloc(sizeof(char)*100);
				memset(random_str,0,sizeof(random_str));
				memset(token,0,sizeof(token));

				randomstr(random_str,strlen(header->Password));
				Tokengenerator(token,RightFlag,random_str);
				sprintf(update_str,
				"INSERT INTO Authentication(RightFlag,LoginName,Token,CreateDate)VALUES('%d','%s','%s',CURRENT_TIMESTAMP)",
				RightFlag,header->Username,token);
				sqlite3_exec(db, update_str, 0, 0, &errMsg);

				free(random_str);
				free(token);
			}
			memset(header_username,0,1024);
			memcpy(header_username,header->Username,strlen(header->Username)+1);
			sqlite3_free_table(result);
			sqlite3_close(db);
			pthread_mutex_unlock(&db_mutex);
			strcpy(Current_User,header->Username);
			return 0;

		}
		else
		{
			sqlite3_free_table(result);
			sqlite3_close(db);
			pthread_mutex_unlock(&db_mutex);
			return -1;
		}
	}
	else
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

}
int CheckPermission()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	char query_str[300]={0};

	pthread_mutex_lock(&db_mutex);

	if (sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}

	sprintf(query_str, "SELECT RightFlag from User WHERE User.LoginName='%s' AND User.DeleteFlag = 0;",Current_User);
	sqlite3_get_table(db,query_str, &result, &rows, &cols, &errMsg);
	if (rows > 0)
	{
		int permission=atoi(*(result + 1));
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return permission;
	}
	else
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		pthread_mutex_unlock(&db_mutex);
		return -2;
	}
}

