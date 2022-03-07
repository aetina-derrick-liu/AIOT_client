#include <cstring>
#include "WebCommand.hpp"
#include "curl/curl.h"
#include "JsonParser.hpp"
#include "LogAgent.hpp"

char WebAddress[MAX_SERVICE_STR_SIZE];
struct MemoryStruct{
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct*)userp;
	
	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL)
	{
#ifdef DEBUG
		LogAgent_Write("not enough memory (realloc returned NULL)");
#endif
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int sendGetRequest(char* cmd, int cmdSize, void* chunk)
{
	CURL *curl_handle;
	CURLcode res;

	
	char* requestURL = (char*)malloc(sizeof(char) * (strlen(WebAddress) + cmdSize + 8));

	sprintf(requestURL, "%s/Device/%s",WebAddress, cmd);

#ifdef DEBUG
	LogAgent_Write("Resquest URL: %s", requestURL);
#endif

	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, requestURL);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, chunk);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	res = curl_easy_perform(curl_handle);

	if(res != CURLE_OK)
	{
		LogAgent_Write((char*)"curl_easy_perform() failed: %s", curl_easy_strerror(res));
	}

	curl_easy_cleanup(curl_handle);

	free(requestURL);

	return 0;
}

int WebCommand_DeviceAuth(char* thumbprint, char* DeviceName, char* PWD)
{
	char* requestURL, *thumbprintHeader;
	const char* responseDevName, *responsePWD;
	CURL *curl_handle;
	CURLcode res;
	struct curl_slist *headers = NULL;
	long http_response_code = 0;
	struct MemoryStruct response;
	struct json_object* jobj, *dev_obj, *pwd_obj;	

	response.memory = (char*)malloc(1);
	response.size = 0;
	
	requestURL = (char*)malloc(sizeof(char) * (strlen(WebAddress) + 30));
	thumbprintHeader = (char*)malloc(sizeof(char) * (strlen(thumbprint) + 13));

	memset(requestURL,0,sizeof(requestURL));
	memset(thumbprintHeader,0,sizeof(thumbprintHeader));
	sprintf(requestURL, "%s/AuthenticationAPI/GetID", WebAddress);
	sprintf(thumbprintHeader, "thumbprint: %s", thumbprint);

#ifdef DEBUG
	LogAgent_Write("Resquest URL: %s", requestURL);
	LogAgent_Write("Get %s", thumbprintHeader);
#endif

	curl_handle = curl_easy_init();
	if(curl_handle == NULL)
	{
		LogAgent_Write((char*)"%s : curl_easy_init() failed.", __func__);
		free(requestURL);
		free(thumbprintHeader);
		free(response.memory);
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();
		curl_slist_free_all(headers);
		return -1;
	}

	curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(curl_handle, CURLOPT_URL, requestURL);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	headers = curl_slist_append(headers, "cache-control: no-cache");
	headers = curl_slist_append(headers, thumbprintHeader);
	headers = curl_slist_append(headers, "content-type: application/x-www-form-urlencoded");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

	res = curl_easy_perform(curl_handle);

	if(res != CURLE_OK)
	{
		LogAgent_Write((char*)"%s : curl_easy_perform() failed: %s.", __func__, curl_easy_strerror(res));
		free(requestURL);
		free(thumbprintHeader);
		free(response.memory);
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();
		curl_slist_free_all(headers);
		return -2;
	}

	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_response_code);

	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	curl_slist_free_all(headers);

#ifdef DEBUG
	LogAgent_Write("Get response: Response status:%ld, Payload:%s", http_response_code, response.memory);
#endif
	if(http_response_code == 200)
	{
		jobj = json_tokener_parse(response.memory);
		dev_obj = get_json_object(jobj, "DeviceId");
		pwd_obj = get_json_object(jobj, "PWD");

		responseDevName = (char*)json_object_get_string(dev_obj);
		responsePWD = (char*)json_object_get_string(pwd_obj);
		memset(DeviceName, 0, MAX_DEVICENAME_SIZE);
		memset(PWD, 0, MAX_PWD_SIZE);
		strncpy(DeviceName, responseDevName, strlen(responseDevName));
		strncpy(PWD, responsePWD, strlen(responsePWD));
		json_object_put(jobj);
#ifdef DEBUG
		LogAgent_Write("Get DevName=%s, PWD=%s", DeviceName,PWD);
#endif
	}

	free(requestURL);
	free(thumbprintHeader);
	free(response.memory);

#ifdef DEBUG
	LogAgent_Write("Free response memory");
#endif

	return 0;
}

