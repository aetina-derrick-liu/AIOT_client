/* Allen 2017.11.24 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CMDParser.h"
#include "NetcardInfo.h"

#ifdef WIN32
#include <iphlpapi.h>
#include <assert.h>
#endif
#ifdef WIN32
static unsigned long get_net_info(IP_ADAPTER_INFO *adapter, unsigned long len)
{
    unsigned long ret;

    ret = GetAdaptersInfo(adapter, &len);    

    return ret;
}
#endif

NETCARD_INFO* lib_SysInfo_GetNetcardInfo()
{
	//NETCARD_INFO* ret = (NETCARD_INFO*)malloc(sizeof(NETCARD_INFO));
    NETCARD_INFO* ret = NULL;
	NETCARD_INFO* prev_node = NULL;
	USHORT i;

#ifdef WIN32    
    IP_ADAPTER_INFO Adapter[16]; 
    IP_ADAPTER_INFO  *pAdapter;

    int len;

    if(!get_net_info(Adapter, sizeof(Adapter)))
    {
        for(i=0, pAdapter=Adapter; pAdapter; i++, pAdapter=pAdapter->Next)
        {
		    NETCARD_INFO* current_node = (NETCARD_INFO*)malloc(sizeof(NETCARD_INFO));

		    current_node->next_node = NULL;

		    if(prev_node != NULL)
			    prev_node->next_node = (void*)current_node;
		    else
			    current_node->prev_node = NULL;

		    current_node->Index       = i;

            len = strlen(pAdapter->Description) + 1;
            current_node->Description = malloc(len);
            memcpy(current_node->Description, pAdapter->Description, len);

            len = strlen(pAdapter->Address) + 1;
            current_node->MACAddress = malloc(len);
            memcpy(current_node->MACAddress, pAdapter->Address, len);

            len = strlen(pAdapter->IpAddressList.IpAddress.String) + 1;
            current_node->IPAddress = malloc(len);
            memcpy(current_node->IPAddress, pAdapter->IpAddressList.IpAddress.String, len);

            len = strlen(pAdapter->IpAddressList.IpMask.String) + 1;
            current_node->IPMask = malloc(len);
            memcpy(current_node->IPMask, pAdapter->IpAddressList.IpMask.String, len);

            len = strlen(pAdapter->GatewayList.IpAddress.String) + 1;
            current_node->Gateway = malloc(len);
            memcpy(current_node->Gateway, pAdapter->GatewayList.IpAddress.String, len);

		    if(prev_node != NULL)
			    current_node->prev_node = (void*)prev_node;
		    else
			    ret = current_node;

		    prev_node = current_node;
        }
    }

#else
#endif
	return ret;
}
