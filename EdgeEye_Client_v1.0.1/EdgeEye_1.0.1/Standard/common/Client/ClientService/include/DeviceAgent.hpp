#ifndef __DEVICEAGENT_HPP__
#define __DEVICEAGENT_HPP__

#include "SocketAgent.hpp"


int DeviceAgent_Start();
int DeviceAgent_Stop();
void DA_receiveGatewayCommand(char* payload, int payloadLength);
void DA_receiveRebootCommand(char* payload, int payloadLength);
void DA_receiveDeviceCommand(SOCKET_DATA* sockdata);
void SendStaticRawData();
void DeviceAgent_StartOffline();
void SendUpdatedAlias(char* new_alias);

#endif
