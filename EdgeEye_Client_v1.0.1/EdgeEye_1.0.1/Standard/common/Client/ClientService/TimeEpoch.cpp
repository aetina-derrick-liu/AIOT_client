#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "TimeEpoch.hpp"
#include <sys/time.h> 
#include <string.h>


#ifdef WIN32
#include <windows.h>
#else
  extern "C"
  {
    #include "CMDParser.h"
  }
#endif

long utc_offset_second(time_t t) {
  struct tm local = *localtime(&t);
  struct tm utc = *gmtime(&t);
  long diff = ((local.tm_hour - utc.tm_hour) * 60 + (local.tm_min - utc.tm_min))
          * 60L + (local.tm_sec - utc.tm_sec);
  int delta_day = local.tm_mday - utc.tm_mday;
  if ((delta_day == 1) || (delta_day < -1)) {
    diff += 24L * 60 * 60;
  } else if ((delta_day == -1) || (delta_day > 1)) {
    diff -= 24L * 60 * 60;
  }
  return diff;
}

long GetEpochDifferenceFromString(char* str)
{
  struct tm t;
  time_t t_of_day = (time_t)(0), t_offset = (time_t)(0);
  char get_date[80]={0}, get_time[80]={0}, *tok;
  int year = 0, month = 0, day = 0, hour = 0 , minute = 0, second = 0;

  long utc_diff_second = utc_offset_second(t_offset);

  sscanf(str, "%s%s", get_date, get_time);

  tok = strtok(get_date, "-");
  year = strtol(tok, NULL, 10);
  tok = strtok(NULL, "-");
  month = strtol(tok, NULL, 10);
  tok = strtok(NULL, "-");
  day = strtol(tok, NULL, 10);

  tok = strtok(get_time, ":");
  hour = strtol(tok, NULL, 10);
  tok = strtok(NULL, ":");
  minute = strtol(tok, NULL, 10);
  tok = strtok(NULL, ":");
  second = strtol(tok, NULL, 10);

  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  t.tm_sec = second + utc_diff_second;
  t.tm_isdst = -1;
  t_of_day = mktime(&t);

  long current_epoch = GetCurrentEpoch();

  return current_epoch - t_of_day;
}

long GetCurrentEpoch()
{
  time_t now;

  time(&now);

  return now;
}

int SetLocalTime(char* Date,char* Time,char* ntpserverip,int timesyncstatus)
{
  char* ret;
  int year=0,month=0,day=0;
  int hour=0,min=0,sec=0;

if( Date != NULL && Time != NULL)
{
  char *delim1="-";
  char *delim2=":";
  char* pch1;
  char* pch2;
  pch1 = strtok(Date,delim1);
  year=atoi(pch1);
  pch1 = strtok(NULL,delim1);
  month=atoi(pch1);
  pch1 = strtok(NULL,delim1);
  day=atoi(pch1);

  pch2 = strtok(Time,delim2);
  hour=atoi(pch2);
  pch2 = strtok(NULL,delim2);
  min=atoi(pch2);
  pch2 = strtok(NULL,delim2);
  sec=atoi(pch2);
}
#ifdef DEBUG
  printf("Set date:%d %d %d,time:%d %d %d\n",year,month,day,hour,min,sec);
#endif

if(timesyncstatus==0)
{
#ifdef WIN32
  int val=0;
  SYSTEMTIME system_time = {0};
  GetLocalTime(&system_time);
  
  system_time.wYear = year;
  system_time.wMonth = month;
  system_time.wDay=day;
  system_time.wHour=hour;
  system_time.wMinute=min;
  system_time.wSecond=sec;
  val = SetLocalTime(&system_time);
  if(0 == val)
  {
    printf("Set local time error!\n");
    return -1;
  }
  else
  {
    printf("Set local time success.\n");
    return 0;
  } 
#else
  char* cmd_ret=(char*)malloc(sizeof(char)*100);
  memset(cmd_ret,0,100);
  GetCommandResult(cmd_ret,"sed -i '/ntpdate/d' /etc/crontab",NULL,0);//delete ntpdate crontab
  GetCommandResult(cmd_ret,"timedatectl set-ntp no",NULL,0);//close ntp sync
  char cmd_buff[1024]={0};
  sprintf((char*)cmd_buff,"date -s \"%02d/%02d/%04d %02d:%02d:%02d\" && hwclock -w",month,day,year,hour,min,sec);
  ret=GetCommandResult(cmd_ret,cmd_buff,NULL,0);
  free(cmd_ret);
  if(ret==NULL)
  {
    printf("Set local time error!\n");
    return -1;
  }
#endif
}
else if(timesyncstatus==1)
{
#ifndef  WIN32
  char* cmd_ret=(char*)malloc(sizeof(char)*100);
  GetCommandResult(cmd_ret,"sed -i '/ntpdate/d' /etc/crontab",NULL,0);//delete old ntpdate crontab
  GetCommandResult(cmd_ret,"timedatectl set-ntp yes",NULL,0);//open ntp sync
  GetCommandResult(cmd_ret,"hwclock -w",NULL,0);
  char cmd_buff[1024]={0};
  sprintf((char*)cmd_buff,"echo \"0 0 * * 0 root (/usr/sbin/ntpdate %s && /sbin/hwclock -w) &> /dev/null\" >> /etc/crontab",ntpserverip);//every week
  ret=GetCommandResult(cmd_ret,cmd_buff,NULL,0);
  free(cmd_ret);
#endif

}
return 0;
}
