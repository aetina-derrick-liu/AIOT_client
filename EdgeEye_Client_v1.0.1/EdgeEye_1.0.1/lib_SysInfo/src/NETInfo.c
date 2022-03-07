#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "CMDParser.h"
#include "NETInfo.h"
#ifdef WIN32
#include <iphlpapi.h>
#endif
#ifndef WIN32

void RemoveCharacter(char* src, const char character)
{
	const char* d = src;
	do {
		while (*d == character)
			++d;
	} while ((*src++ = *d++));
}

USHORT GetNumberOfNETCard()
{
	char* ret;
	USHORT retValue;

	ret = MEMAllocting(__func__);	
		
	GetCommandResult(ret, "find /sys/class/net -type l -not -lname '*virtual*' | wc -l", NULL, 0);
	retValue = atoi(ret);

	free(ret);	

	return retValue;
}

char* GetNETName(USHORT Index)
{
	char* ret, *cmd, *path;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
	path = MEMAllocting(__func__);	

	sprintf(cmd, "find /sys/class/net -type l -not -lname '*virtual*' | tail -n +%u | awk '{print $1}'", Index + 1);
	GetCommandResult(path, cmd,"", 0);
	sprintf(ret, "%s", basename(path));
	
	free(cmd);
	free(path);
	
	return MEMReallocting(ret);
}

char* GetNETType(char* Name)
{
	char* ret, *cmd, *path;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
	
	sprintf(cmd, "nmcli -m tabular -t -f general.type device show %s", Name);

	GetCommandResult(ret, cmd, "", 0);

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETManufacturer(USHORT Index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "lshw -class network | grep vendor | tail -n +%u", Index + 1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETProductName(USHORT Index)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "lshw -class network | grep product | tail -n +%u", Index + 1);

	GetCommandResult(ret, cmd, ":", 1);

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETMACAddress(char* Name)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "nmcli -m tabular -t -f general.hwaddr device show %s", Name);

	GetCommandResult(ret, cmd, "", 0);

	RemoveCharacter(ret, '\\');

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETIPv4Address(char* Name)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "nmcli -m tabular -t -f ip4.address device show %s", Name);

	GetCommandResult(ret, cmd, "", 0);

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETNetmask(char* Name)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);

#ifdef __aarch64__
	sprintf(cmd, "ifconfig %s | awk \"/netmask/{print}\"", Name);
    GetCommandResult(ret, cmd, " ", 3);
#else
    sprintf(cmd, "ifconfig %s | awk \"/Mask:/{print}\"", Name);
    GetCommandResult(ret, cmd, " :", 6);
#endif

	free(cmd);
	
	return MEMReallocting(ret);
}

char* GetNETIPv6Address(char* Name)
{
	char* ret, *cmd;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);	
	
	sprintf(cmd, "nmcli -m tabular -t -f ip6.address device show %s", Name);

	GetCommandResult(ret, cmd, "", 0);

	RemoveCharacter(ret, '\\');

	free(cmd);
	
	return MEMReallocting(ret);
}
#else
unsigned long get_net_info(IP_ADAPTER_INFO *adapter, unsigned long len)
{
    unsigned long ret;

    ret = GetAdaptersInfo(adapter, &len);    

    return ret;
}
#endif

NET_INFO* lib_SysInfo_GetNETInfo()
{
	NET_INFO* ret = NULL;
	NET_INFO* prev_node = NULL;
	USHORT i;

#ifdef WIN32
	// free(ret);
	// return NULL;
	IP_ADAPTER_INFO Adapter[16]; 
    IP_ADAPTER_INFO  *pAdapter;

	int len;

	if(!get_net_info(Adapter, sizeof(Adapter)))
	{
		for(i=0, pAdapter=Adapter; pAdapter; i++, pAdapter=pAdapter->Next)
		{
			NET_INFO* current_node = (NET_INFO*)malloc(sizeof(NET_INFO));
			current_node->next_node = NULL;
			if(prev_node != NULL)
			{
				prev_node->next_node = (void*)current_node;
			}
			else
			{
				current_node->prev_node = NULL;
			}
			current_node->Index = i;
			
			len = strlen(pAdapter->Description) + 1;
			current_node->Name = (char*)malloc(sizeof(char) * len);
			memcpy(current_node->Name, pAdapter->Description, len);
			*(current_node->Name+(len - 1)) = '\0';

			switch(pAdapter->Type)
			{
				case MIB_IF_TYPE_ETHERNET:
					current_node->Type = (char*)malloc(sizeof(char) * 9);
					sprintf(current_node->Type, "Ethernet");
					break;
				case IF_TYPE_ISO88025_TOKENRING:
					current_node->Type = (char*)malloc(sizeof(char) * 10);
					sprintf(current_node->Type, "TokenRing");
					break;
				case MIB_IF_TYPE_PPP:
					current_node->Type = (char*)malloc(sizeof(char) * 15);
					sprintf(current_node->Type, "Point-to-Point");
					break;
				case MIB_IF_TYPE_LOOPBACK:
					current_node->Type = (char*)malloc(sizeof(char) * 9);
					sprintf(current_node->Type, "Loopback");
					break;
				case MIB_IF_TYPE_SLIP:
					current_node->Type = (char*)malloc(sizeof(char) * 5);
					sprintf(current_node->Type, "SLIP");
					break;
				case IF_TYPE_IEEE80211:
					current_node->Type = (char*)malloc(sizeof(char) * 9);
					sprintf(current_node->Type, "Wireless");
					break;
				default:
					current_node->Type = (char*)malloc(sizeof(char) * 8);
					sprintf(current_node->Type, "Unknown");
					break;
			}

			current_node->MACAddress = (char*)malloc(sizeof(char) * 18);
			sprintf(current_node->MACAddress, "%02X:%02X:%02X:%02X:%02X:%02X",
					*(pAdapter->Address+0),
					*(pAdapter->Address+1),
					*(pAdapter->Address+2),
					*(pAdapter->Address+3),
					*(pAdapter->Address+4),
					*(pAdapter->Address+5));

			len = strlen(pAdapter->IpAddressList.IpAddress.String) + 1;
			current_node->IPv4Address = (char*)malloc(sizeof(char) * len);
			memcpy(current_node->IPv4Address, pAdapter->IpAddressList.IpAddress.String, len);
			*(current_node->IPv4Address+(len - 1)) = '\0';

			len = strlen(pAdapter->IpAddressList.IpMask.String) + 1;
			current_node->Netmask = (char*)malloc(sizeof(char) * len);
			memcpy(current_node->Netmask, pAdapter->IpAddressList.IpMask.String, len);
			*(current_node->Netmask+(len - 1)) = '\0';

			current_node->IPv6Address = (char*)malloc(sizeof(char) * 1);
			*(current_node->IPv6Address + 0) = '\0';

			if(prev_node != NULL)
			{
				current_node->prev_node = (void*)prev_node;
			}
			else
			{
				ret = current_node;
			}
			prev_node = current_node;
		}
	}

#else
	for(i = 0; i < GetNumberOfNETCard(); i++)
	{
		NET_INFO* current_node = (NET_INFO*)malloc(sizeof(NET_INFO));
		current_node->next_node = NULL;
		if(prev_node != NULL)
		{
			prev_node->next_node = (void*)current_node;
		}
		else
		{
			current_node->prev_node = NULL;
		}
		current_node->Index = i;
		current_node->Name = GetNETName(i);
		current_node->Type = GetNETType(current_node->Name);
		//current_node->Manufacturer = GetNETManufacturer(i);
		//current_node->ProductName = GetNETProductName(i);
		current_node->MACAddress = GetNETMACAddress(current_node->Name);
		current_node->IPv4Address = GetNETIPv4Address(current_node->Name);
		current_node->Netmask = GetNETNetmask(current_node->Name);
		current_node->IPv6Address = GetNETIPv6Address(current_node->Name);
		if(prev_node != NULL)
		{
			current_node->prev_node = (void*)prev_node;
		}
		else
		{
			ret = current_node;
		}
		prev_node = current_node;
	}
#endif
	return ret;
}

