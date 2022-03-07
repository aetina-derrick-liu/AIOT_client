#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Base64.hpp"
#include "TimeEpoch.hpp"

char *randomstr(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

int Tokengenerator(char* token,int rightflag,char* randomstr)
{
    char* srcstr=(char*)malloc(sizeof(char)*100);
    memset(srcstr,0,sizeof(srcstr));

    long currtime=GetCurrentEpoch();
    sprintf(srcstr,"%d#%s#%ld",rightflag,randomstr,currtime);
    base64_encode((unsigned char*)srcstr,
				  (unsigned char*)token,
				  strlen(srcstr),
				  0);
    free(srcstr);
    return 0;
}