#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "Crypto.hpp"
#include "main.hpp"
#include "MQTTAgent.hpp"	
#include "MQTTAsync.h"
#include "DeviceAgent.hpp"
#include "LogAgent.hpp"

using namespace std;

#define QOS		1
#define TIMEOUT		10000L

Crypto crypto;

volatile MQTTAsync_token deliveredtoken;

int disc_finished = 0;
int finished = 0;
int subscribed = 0;

static MQTTAsync client;
char GatewayAddress[MAX_SERVICE_STR_SIZE],
	 DeviceName[MAX_DEVICENAME_SIZE],
	 PWD[MAX_PWD_SIZE];
int (*DA_sendGatewayCommand)(char* topic, const char*);

void printBytesAsHex(unsigned char *bytes, size_t length, const char *message)
{
	printf("%s: ", message);

	for (unsigned int i = 0; i < length; i++) {
#ifdef WIN32
		__mingw_printf("%02hhx ", bytes[i]);
#else
		printf("%02hhx ", bytes[i]);
#endif
	}

	puts("");
}

int onMessageArrive(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
	// printBytesAsHex(static_cast<unsigned char*>(message->payload), message->payloadlen, "onMessageArrive");

	const char* payloadptr = (const char*)message->payload;

	string tempstr;
	for (int i = 0; i < message->payloadlen; i++)
	{
		tempstr = tempstr + *payloadptr++;
	}

	secure_string ctext(tempstr.data(), message->payloadlen);
	secure_string rtext;

	string topic;
	topic.assign(topicName);
	if (topic.compare("Remote") == 0) {	
		DA_receiveRebootCommand((char*)tempstr.data(), message->payloadlen);
	}

	if (crypto.aes_decrypt(ctext, rtext) != 0)
	{
		cout << "MQTT payload decryption failed. Drop the payload!" << endl;
		return -1; 
	}

#ifdef DEBUG
	LogAgent_Write("Receive data from topic:%s", topicName);
	LogAgent_Write("---Recv raw---");
	LogAgent_Write("%s", rtext.c_str());
	LogAgent_Write("--------------");
	LogAgent_Write("Length: %d", message->payloadlen);
	LogAgent_Write("---- Done ----");
#endif

	DA_receiveGatewayCommand((char*)rtext.c_str(), rtext.length());

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
#ifdef DEBUG
	LogAgent_Write("Subscribe succeeded");
#endif

	subscribed = 1;
	finished = 0;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
#ifdef DEBUG
	LogAgent_Write("Subscribe failed, rc %d", response ? response->code : 0);
#endif

	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

#ifdef DEBUG
	LogAgent_Write("Successful connection");
	LogAgent_Write("Subscribing to topic %s for client %s using QoS%d", TOPIC_CMD, DeviceName, QOS);
	LogAgent_Write("Subscribing to topic %s for client %s using QoS%d", TOPIC_REM, DeviceName, QOS);
	LogAgent_Write("Subscribing to topic %s for client %s using QoS%d", TOPIC_ATV, DeviceName, QOS);
#endif
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;

	deliveredtoken = 0;

	if((rc = MQTTAsync_subscribe(client, TOPIC_CMD, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start subscribe topic %s, return code %d", TOPIC_CMD, rc);
#endif
	}
	if((rc = MQTTAsync_subscribe(client, TOPIC_REM, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start subscribe topic %s, return code %d", TOPIC_REM, rc);
#endif
	}
	if((rc = MQTTAsync_subscribe(client, TOPIC_ATV, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start subscribe topic %s, return code %d", TOPIC_ATV, rc);
#endif
	}
	/* tina add 
	if((rc = MQTTAsync_subscribe(client, TOPIC_REM_NATIVE, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start subscribe topic %s, return code %d", TOPIC_REM, rc);
#endif
	}*/
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
#ifdef DEBUG
	LogAgent_Write("Connect failed, rc %d", response ? response->code : 0);
#endif
	finished = 1;
	ServiceStatus = 1;
}

void onConnectLost(void* context, char* cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	LogAgent_Write("Connect lost, cause: %s", cause);
#ifdef DEBUG
	LogAgent_Write("Trying to Reconnection");
#endif

	if(cause != NULL)
	{
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
		if((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
		{
			LogAgent_Write("Failed to reconnect, return code %d", rc);

			finished = 1;
		}
	}
	else
	{
		LogAgent_Write("Gateway service was shutdown.");
		finished = 1;
	}
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
#ifdef DEBUG
	LogAgent_Write("Successful disconnection.");
#endif
	disc_finished = 1;
}

void onSend(void* context, MQTTAsync_successData* response)
{
#if 0
	printf("Send message success, token value %d delivery confirmed, payload reference:%d\n", response->token, &(response->alt.pub.message.payload));
#endif
}

int MQTTAgent_PublishData(char* topic, const char* payload)
{	
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	opts.onSuccess = NULL;
	opts.context = client;

	secure_string ctext;
	if (crypto.aes_encrypt(payload, ctext) != 0)
	{
		cout << "MQTT payload encryption failed." << endl;
		return -1;
	}

	unsigned char send[ctext.length()];
   	std::copy(ctext.begin(), ctext.end(), send);
	
	// printBytesAsHex(send, sizeof(send), "MQTTAgent_PublishData");

	pubmsg.payload = static_cast<void*>(send);
	pubmsg.payloadlen = sizeof(send);

#ifdef DEBUG
	LogAgent_Write("Publish data to topic %s, strlen=%d", topic, pubmsg.payloadlen);
	LogAgent_Write("---Send raw---");
	if (pubmsg.payloadlen <= MAX_LOG_STR_SIZE)
		LogAgent_Write("%s", (char*)payload);
	LogAgent_Write("--------------");
	LogAgent_Write("Length: %d", pubmsg.payloadlen);
	LogAgent_Write("---- Done ----");
#endif

	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	deliveredtoken = 0;

	if(finished)
	{
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
		conn_opts.onSuccess = onConnect;
		conn_opts.onFailure = onConnectFailure;
		conn_opts.context = client;
		conn_opts.username = DeviceName;
		conn_opts.password = PWD;

		if((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
		{
			return -3;
		}

		while(!subscribed)
		{
			usleep(10000L);
			if(finished)
			{
				break;
			}
		}

		if(finished)
		{
			return -4;
		}

	}

	if((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start sendMessage, return code %d", rc);
#endif
	}

	//printf("Send token=%d\n", rc);

	return rc;
}

void MQTTAsync_Complete(void *context, MQTTAsync_token token)
{
#if 0
	printf("Completed, token=%d.\n", token);
#endif
}

int MQTTAgent_Start()
{

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	rc = MQTTAsync_create(&client, GatewayAddress, DeviceName, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	if(rc != MQTTASYNC_SUCCESS)
	{
		return -1;
	}

	rc = MQTTAsync_setCallbacks(client, NULL, onConnectLost, onMessageArrive, MQTTAsync_Complete);

	if(rc != MQTTASYNC_SUCCESS)
	{
		return -2;
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	conn_opts.username = DeviceName;
	conn_opts.password = PWD;

	DA_sendGatewayCommand = MQTTAgent_PublishData;

	if((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		return -3;
	}

	subscribed = 0;
	finished = 0;

	while(!subscribed)
	{
		usleep(10000L);
		if(finished)
		{
			break;
		}
	}

	if(finished)
	{
		return -4;
	}

	return 0;
}

void MQTTAgent_Stop()
{
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	int rc;
	disc_opts.onSuccess = onDisconnect;

	if((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
#ifdef DEBUG
		LogAgent_Write("Failed to start disconnect, return code %d", rc);
#endif
		MQTTAgent_Destroy();
		return;
	}

	while(!disc_finished)
	{
		usleep(10000L);
	}

	MQTTAgent_Destroy();
}

void MQTTAgent_Destroy()
{
	MQTTAsync_destroy(&client);
}
