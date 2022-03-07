#ifndef __TIMEEPOCH_HPP__
#define __TIMEEPOCH_HPP__

long GetEpochDifferenceFromString(char* str);
long GetCurrentEpoch();

int SetLocalTime(char* Date,char* Time,char* ntpserverip,int timesyncstatus);

#endif