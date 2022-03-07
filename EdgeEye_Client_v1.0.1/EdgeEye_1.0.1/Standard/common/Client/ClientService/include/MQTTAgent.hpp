#ifndef __MQTTAGENT_HPP__
#define __MQTTAGENT_HPP__

#include "ClientService.hpp"

#define TOPIC_CMD "Command"
#define TOPIC_RAW "RawData"
#define TOPIC_REM "Remote"
#define TOPIC_ATV "ActiveMsg"
// tina #define TOPIC_REM_NATIVE "RemoteNative"


int MQTTAgent_PublishData(char* topic, const char* payload);
int MQTTAgent_Start();
void MQTTAgent_Stop();
void MQTTAgent_Destroy();
#endif
