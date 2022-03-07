#include <iostream>
#include <pthread.h>
#include <time.h>
 #include <unistd.h>
#include "DeviceAgent.hpp"
#include "MQTTAgent.hpp"
#include "JsonParser.hpp"
#include "DataBuilder.hpp"
#include "DBInterface.hpp"
#include "screenshot.hpp"
#include "TimeEpoch.hpp"
#include "LogAgent.hpp"
#include <sstream>
#include <string.h>
using namespace std ;


char programState = 0, offlineStatus = 1;
unsigned long Interval = 60000L;

#ifdef WIN32
static clock_t DYRowTimer;
#else
static double DYRowTimer;
#endif

pthread_t pth;
extern char GatewayAddress[MAX_SERVICE_STR_SIZE],
		DeviceName[MAX_DEVICENAME_SIZE],
		PWD[MAX_PWD_SIZE];
extern int (*DA_sendGatewayCommand)(char* topic, const char*);

void SendReg()
{
	struct json_object* jobj = json_object_new_object();

#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send register command to core service.");
#endif	

	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "Cmd", json_object_new_string("reg"));
	(*DA_sendGatewayCommand)(TOPIC_CMD,json_object_to_json_string(jobj));
	json_object_put(jobj);
}

void SendOnline()
{
	struct json_object* jobj = json_object_new_object();
	
#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send online command to core service.");
#endif	
	
	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "Cmd", json_object_new_string("online"));
	(*DA_sendGatewayCommand)(TOPIC_CMD, json_object_to_json_string(jobj));
	json_object_put(jobj);
}

void SendBusy()
{
	struct json_object* jobj = json_object_new_object();
	
#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send busy command to core service.");
#endif	
	
	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "Cmd", json_object_new_string("busy"));
	(*DA_sendGatewayCommand)(TOPIC_CMD, json_object_to_json_string(jobj));
	json_object_put(jobj);
}

void SendOffline()
{
	struct json_object* jobj = json_object_new_object();
	
#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send offline command to core service.");	
#endif	
	
	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "Cmd", json_object_new_string("offline"));
	(*DA_sendGatewayCommand)(TOPIC_CMD, json_object_to_json_string(jobj));
	json_object_put(jobj);
}

void SendGetRemoteCmd()
{
	struct json_object* jobj = json_object_new_object();
	
#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send remote command to core service.");
#endif	
	
	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "remote", json_object_new_string("ok"));
	(*DA_sendGatewayCommand)(TOPIC_REM, json_object_to_json_string(jobj));
	json_object_put(jobj);
}
void SendUpdatedAlias(char* new_alias)
{
	struct json_object* jobj = json_object_new_object();

#ifdef DEBUG
	LogAgent_Write("[Gateway:Send] Send updated alias to core service.");
#endif	
	json_object_object_add(jobj, "ID", json_object_new_string(DeviceName));
	json_object_object_add(jobj, "Alias", json_object_new_string(new_alias));
	(*DA_sendGatewayCommand)(TOPIC_ATV,json_object_to_json_string(jobj));
	json_object_put(jobj);
}


#ifdef WIN32
double diffclock(clock_t clock1,clock_t clock2)
#else
double diffclock(double clock1,double clock2)
#endif
{
	double diffticks=clock1-clock2;
#ifdef WIN32
	double diffms=(diffticks)/(CLOCKS_PER_SEC) * 1000;
#else
	double diffms=(diffticks) * 1000;
#endif
	return diffms;
}

void* push_data_contiune(void* ptr)
{
	int ret;
#ifdef WIN32
	clock_t now = clock();
	DYRowTimer = clock();
#else
	double now = time(NULL);
	DYRowTimer = time(NULL);
#endif
	struct json_object* obj;
	obj = GetStaticRawData();

#ifndef DAEMON
	LogAgent_Write((char*)"[Gateway:Send] Send static raw data to core service.");
#endif
	(*DA_sendGatewayCommand)(TOPIC_RAW, json_object_to_json_string(obj));
	ServiceStatus = 0;
	json_object_put(obj);
	do
	{
		obj = GetDynamicRawData();

#ifndef DAEMON
		LogAgent_Write((char*)"[Gateway:Send] Send dynamic raw data to core service.");
#endif
		ret = (*DA_sendGatewayCommand)(TOPIC_RAW, json_object_to_json_string(obj));
		json_object_put(obj);

		while(diffclock(now, DYRowTimer) < Interval)
		{
#ifdef WIN32
			Sleep(100);
			now = clock();
#else
			usleep(100000L);
			now = time(NULL);
#endif
		}
#ifdef WIN32
		DYRowTimer = clock();
#else
		DYRowTimer = time(NULL);
#endif

		if(ServiceStatus != 0)
		{
			programState = 0;
			return 0;
		}
	}while(programState);
}

void* get_raw_data_contiune(void* ptr)
{
	pthread_detach(pthread_self());
	int ret;
#ifdef WIN32
	clock_t now = clock();
	DYRowTimer = clock();
#else
	double now = time(NULL);
	DYRowTimer = time(NULL);
#endif
	do
	{
#ifndef DAEMON		
		LogAgent_Write((char*)"Get local dynamic raw data");
#endif
		GetLocalDynamicRawData(NULL);

		while(diffclock(now, DYRowTimer) < 10000)
		{
#ifdef WIN32
			Sleep(100);
			now = clock();
#else
			usleep(100000L);
			now = time(NULL);
#endif
		}
#ifdef WIN32
		DYRowTimer = clock();
#else
		DYRowTimer = time(NULL);
#endif
	}while(offlineStatus != 0);
#ifndef DAEMON
	LogAgent_Write((char*)"Start publish data to iCAP server, stop offline process.\n");
#endif
}

void SendStaticRawData()
{
	if(ServiceStatus == 0)
	{
		struct json_object* obj;
		obj = GetStaticRawData();
		(*DA_sendGatewayCommand)(TOPIC_RAW, json_object_to_json_string(obj));
		json_object_put(obj);
	}
	DYRowTimer = clock();
}

void SendDynamicRawData()
{
	if(ServiceStatus == 0)
	{
		struct json_object* obj;
		obj = GetDynamicRawData();
		(*DA_sendGatewayCommand)(TOPIC_RAW, json_object_to_json_string(obj));
		json_object_put(obj);
	}
	DYRowTimer = clock();
}

int DeviceAgent_Start()
{
	WriteStatus(2);
	SendReg();
	ServiceStatus = 2;
	return 1;
}

int DeviceAgent_Stop()
{
	if(ServiceStatus == 0)
	{
		SendOffline();
	}
	programState = 0;
	ServiceStatus = 4;
	return 1;
}
/************************************************************/
#ifdef WIN32
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
wstring widen( const string& str )
{
    wostringstream wstm ;
    const ctype<wchar_t>& ctfacet = 
                        use_facet< ctype<wchar_t> >( wstm.getloc() ) ;
    for( size_t i=0 ; i<str.size() ; ++i ) 
              wstm << ctfacet.widen( str[i] ) ;
    return wstm.str() ;
}
#endif
/*****************************************************************/
//{"Cmd":"RemoteNative","ID":"Device00001","Remote":{"Value":"RemoteNative"}}
void DA_receiveRebootCommand(char* payload, int payloadLength)
{
	struct json_object *root, *ID, *Cmd, *Remote, *Value;
	struct json_tokener* tokener;

	tokener = json_tokener_new();

	root = json_tokener_parse_ex(tokener, payload, payloadLength);
	ID = get_json_object(root, "ID");
	Cmd = get_json_object(root, "Cmd");
	Remote = get_json_object(root, "Remote");
	Value = get_json_object(Remote, "Value");

	cout << json_object_get_string(ID) << endl;
	cout << DeviceName << endl;

	if(ID && Cmd && Remote && Value)
	{
		if(strcmp(json_object_get_string(ID), DeviceName) == 0)
		{
			if(strcmp(json_object_get_string(Value), "RemoteNative") == 0)
			{
				cout << "RemoteNative and DeviceAgent_Stop" << endl;
				DeviceAgent_Stop();
				sync();
				sleep(5);
				system("reboot");
			}
		}
	}
}

void DA_receiveGatewayCommand(char* payload, int payloadLength)
{
	struct json_object *root, *ID, *Cmd, *Remote;
	struct json_tokener* tokener;

	tokener = json_tokener_new();

	root = json_tokener_parse_ex(tokener, payload, payloadLength);

#ifdef DEBUG
	if(root==NULL)
	{
		LogAgent_Write("[Gateway:Recevice] Get invalid message, drop.");
	}
	else
#endif
	{
		ID = get_json_object(root, "ID");
		Cmd = get_json_object(root, "Cmd");
		//LogAgent_Write("ID: %s", json_object_get_string(ID));
		//LogAgent_Write("Cmd: %s", json_object_get_string(Cmd));
		//LogAgent_Write("DeviceName: %s", DeviceName);
		if(ID)
		{
			if(strcmp(json_object_get_string(ID), DeviceName) == 0)
			{
				//LogAgent_Write("IN ID == DeviceName");
				if(Cmd)
				{
					WriteStatus(0);
					if(strncmp(json_object_get_string(Cmd), "Ok", 2) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Core service send ok command.");
#endif
					}
					if(strncmp(json_object_get_string(Cmd), "Fail", 4) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Core service send fail command.");
#endif
					}
					if(strncmp(json_object_get_string(Cmd), "status", 6) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Core service send status command.");
#endif
						if(programState == 0)
						{
							SendOnline();
						}
						else
						{
							SendBusy();
						}
					}
					if(strncmp(json_object_get_string(Cmd), "start", 5) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Core service send start command.");
#endif
						if(programState == 0)
						{
							programState = 1;
							pthread_t pth_p;
							pthread_attr_t attr;
							pthread_attr_init (&attr);
							pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
							pthread_create(&pth, &attr, &push_data_contiune, NULL);
							pthread_attr_destroy (&attr);
							offlineStatus = 0;
						}
					}
					//{"Cmd":"RemoteNative","ID":"Device00001","Remote":{"Value":"RemoteReboot"}}
					if(strncmp(json_object_get_string(Cmd), "RemoteReboot", 12) == 0)
					{
						LogAgent_Write("--[aetina Gateway:Recevice] Web service send RemoteReboot command.");
						// Remote = get_json_object(root, "RemoteReboot");
						//{"Cmd":"Remote","ID":"Device00037","Remote":{"Name":"Device00037","Value":"test cmd"}}

					}
					if(strncmp(json_object_get_string(Cmd), "RemoteNative", 12) == 0)
					{
						LogAgent_Write("---[aetina Gateway:Recevice] Web service send RemoteNative command.");
						Remote = get_json_object(root, "Remote");
						struct json_object* value = get_json_object(Remote, "Value");
						//{"Cmd":"Remote","ID":"Device00037","Remote":{"Name":"Device00037","Value":"test cmd"}}
						//LogAgent_Write("--[aetina Gateway:Recevice] Web service send RemoteReboot command.");
						LogAgent_Write("the value is %s", json_object_get_string(value));
					}
					
					if(strncmp(json_object_get_string(Cmd), "Remote", 5) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Web service send remote command.");
#endif
						LogAgent_Write("[aetina Gateway:Recevice] Web service send remote command.");
						Remote = get_json_object(root, "Remote");
						if(SA_SendRemoteCmd((char*)json_object_get_string(get_json_object(Remote, "Name")),
											(char*)json_object_get_string(get_json_object(Remote, "Value"))) == 0)
						{
							SendGetRemoteCmd();
						}
					}
					if(strncmp(json_object_get_string(Cmd), "ScreenShot",10) == 0)
					{
#ifdef DEBUG
						LogAgent_Write("[Gateway:Recevice] Web service send screenshot command.");
#endif
						char* JPG_ = (char*)malloc(sizeof(char)*50);
						wchar_t JPG_WC[100]={0};
						memset(JPG_,0,50);
						sprintf(JPG_,"%s_%ld.jpg",DeviceName,GetCurrentEpoch());
#ifdef WIN32
						TCHAR windir[MAX_PATH];
						POINT start,end;
						start.x=0;
						start.y=0;

						end.x=GetSystemMetrics(SM_CXSCREEN);
						end.y=GetSystemMetrics(SM_CYSCREEN);

	#ifdef DEAMON
						if(IsWow64())
						{
							GetSystemWow64Directory(windir, MAX_PATH);
						}	
						else
						{
							GetSystemDirectory(windir, MAX_PATH);
						}
						char BMP_path[100] = {0};
						char JPG_path[100] = {0};
						char windir_C[MAX_PATH] = {0};
						char JPG_C[150]={0};
						sprintf(windir_C,"%s", windir);
						sprintf(BMP_path,"%s%s",windir_C,"\\Innodisk\\iCAP_ClientService\\tmp.bmp");
						sprintf(JPG_path,"%s%s",windir_C,"\\Innodisk\\iCAP_ClientService\\tmp.jpg");
						sprintf(JPG_C,"%s%s%s",windir_C,"\\Innodisk\\iCAP_ClientService\\",JPG_);
						screenshot(BMP_path);
						BMP2JPG(BMP_path,JPG_path);
						swprintf(JPG_WC, strlen(JPG_C)+1, L"%hs", JPG_C);
						CompressImagePixel(widen(JPG_path).c_str(),JPG_WC,(end.x)/3,(end.y)/3);

	#else

						screenshot("tmp.bmp");
						BMP2JPG("tmp.bmp","tmp.jpg");
						swprintf(JPG_WC, strlen(JPG_), L"%hs", JPG_);
						CompressImagePixel(L"tmp.jpg",JPG_WC,(end.x)/3,(end.y)/3);
	#endif

#else
						char JPG_path[100] = {0};
						sprintf(JPG_path,"%s%s","/var/iCAP_Client/",JPG_);
						int ret=ScrotCmd(JPG_path);
						char* JPG_THU = (char*)malloc(sizeof(char)*50);
						memset(JPG_THU,0,50);
						sprintf(JPG_THU,"%s%s_%ld-thumb.jpg","/var/iCAP_Client/",DeviceName,GetCurrentEpoch());
						swprintf(JPG_WC, strlen(JPG_THU)+1, L"%hs", JPG_THU);
						(ret==0)?LogAgent_Write("Device take screenshot successfully!\n"):LogAgent_Write("Device take screenshot failed.\n");
						free(JPG_THU);
#endif
						struct json_object* obj=GetScreenshot(JPG_WC);
						(*DA_sendGatewayCommand)(TOPIC_RAW, json_object_to_json_string(obj));
						free(JPG_);

					}
				} else {
					printf("no cmd");
				}
			}
		}
#ifdef DEBUG
		else
		{
			LogAgent_Write("[Gateway:Recevice] Data doesnot contain ID, drop.");
		}
#endif
	}
	json_object_put(root);
	json_tokener_free(tokener);
}

void DA_receiveDeviceCommand(SOCKET_DATA* sockdata)
{
	struct json_object* jobj, *cmd, *name, *unit, *type, *info;
	int ret;
	jobj = json_tokener_parse(sockdata->payload);

	pthread_t self = pthread_self();

	if(jobj==NULL)
	{
#ifdef DEBUG
		LogAgent_Write("[Socket:Recevice] Get invalid message, drop.");
#endif
	}
	else
	{
		cmd = get_json_object(jobj, "Cmd");
		if(cmd)
		{
			if(strcmp(json_object_get_string(cmd), "AddExt") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice:%d] Recevice add external sensor command.", self);
#endif
				name = get_json_object(jobj, "Name");
				unit = get_json_object(jobj, "Unit");
				type = get_json_object(jobj, "Type");
				ret = AddExternalSensor(
					(char*)json_object_get_string(name),
					(char*)json_object_get_string(unit),
					(unsigned char)json_object_get_int(type));
				if(ret == 0)
				{
					InsertDataDefine((char*)json_object_get_string(name), (char*)"{\"Dynamic\":{\"Ext\":{\"0\":0}}}", 1, 7);
					int Dataid=GetExtDataid((char*)json_object_get_string(name));
					InsertExtData(Dataid,(char*)json_object_get_string(unit));
					SA_SendOKCmd(sockdata);
					SendStaticRawData();
				}
				else
				{
					SA_SendFailCmd(sockdata);
				}

			}
			else if(strcmp(json_object_get_string(cmd), "DelExt") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice delete external sensor command.");
#endif
				name = get_json_object(jobj, "Name");
				ret = RemoveExternalSensor((char*)json_object_get_string(name));
				if(ret == 0)
				{
					SA_SendOKCmd(sockdata);
					SendStaticRawData();
				}
				else
				{
					SA_SendFailCmd(sockdata);
				}
			}
			else if(strcmp(json_object_get_string(cmd), "AddRemote") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice add remote device command.");
#endif
				name = get_json_object(jobj, "Name");
				unit = get_json_object(jobj, "Unit");
				type = get_json_object(jobj, "Type");
				ret = AddRemoteDevice(
					(char*)json_object_get_string(name),
					(char*)json_object_get_string(unit),
					(unsigned char)json_object_get_int(type));
				if(ret == 0)
				{
					SA_SendOKCmd(sockdata);
					SendStaticRawData();
				}
				else
				{
					SA_SendFailCmd(sockdata);
				}
			}
			else if(strcmp(json_object_get_string(cmd), "DelRemote") == 0)
			{
				name = get_json_object(jobj, "Name");
				ret = RemoveRemoteDevice((char*)json_object_get_string(name));
				if(ret == 0)
				{
					SA_SendOKCmd(sockdata);
					SendStaticRawData();
				}
				else
				{
					SA_SendFailCmd(sockdata);
				}
			}
			else if(strcmp(json_object_get_string(cmd), "ChkStatus") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice check service status command.");
#endif
				SA_SendStatus(sockdata);
			}
			else if(strcmp(json_object_get_string(cmd), "SetInt") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Set interval command.");
#endif
				type = get_json_object(jobj, "Value");
				int time = json_object_get_int(type);
				if(time > 0)
				{
					Interval = time * 1000;
					SA_SendOKCmd(sockdata);
				}
				else
				{
					SA_SendFailCmd(sockdata);
				}
			}
			else if(strcmp(json_object_get_string(cmd), "EventTrigger") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice event trigger command.");
#endif
					SA_SendOKCmd(sockdata);
				  SendDynamicRawData();
			}
			else if(strcmp(json_object_get_string(cmd), "ServiceLogout") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice service logout command.");
#endif
				SA_SendOKCmd(sockdata);
				DeviceLogout();
			}
			else if(strcmp(json_object_get_string(cmd), "AddStatic") == 0)
			{
#ifdef DEBUG
				LogAgent_Write("[Socket:Recevice] Recevice add static info command.");
#endif
				SA_SendOKCmd(sockdata);
				name = get_json_object(jobj,"Name");
				info = get_json_object(jobj,"Info");
#ifdef SG
				UpdateBattID((char*)json_object_get_string(info));
#endif
			}
		}
		else
		{
#ifdef DEBUG
			LogAgent_Write("[Socket:Recevice] Get invalid message, drop.");
#endif
		}
	}
	json_object_put(jobj);
}

void DeviceAgent_StartOffline()
{
	pthread_t pth_p;
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&pth_p, &attr, &get_raw_data_contiune, NULL);
	pthread_attr_destroy (&attr);
}
