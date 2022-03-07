#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lib_SysInfo.h"
#include "CMDParser.h"

#ifdef WIN32
#include <wbemidl.h>
#include <wbemcli.h>
#include <windows.h>
#endif

char* lib_SysInfo_TrimWhiteSpace(char* str)
{
	char* end;
	if(str==NULL)
	{
		return NULL;
	}
	else
	{
		// Trim leading space
		while(isspace((unsigned char)*str)) str++;

		if(*str == 0) // All spaces
		{
			return str;
		}

		// Trim trailing space
		end = str + strlen(str) - 1;
		while(end > str && isspace((unsigned char)*end)) end--;

		// Write new null terminator
		*(end + 1) = 0;

		return str;
	}
}

char* GetCommandResult(char* ret, char* cmd, char* splitStr, int index)
{
	char *buf, *pch, *firstline, *retPointer = NULL;
	int c;
	FILE* fp;
	if(ret == NULL)
	{
		fprintf(stderr, "[%s]Return pointer is null.\n", __func__);
		return NULL;
	}

	if(cmd == NULL)
	{
		fprintf(stderr, "[%s]Command is null.\n", __func__);
		return NULL;
	}

	fp = popen(cmd, "r");

	if(fp == NULL)
	{
		fprintf(stderr, "[%s]popen fail!\n", __func__);
		return NULL;
	}
	buf = (char*)malloc(sizeof(char) * MAX_CMD_STR_LEN);
	if(buf == NULL)
	{
		fprintf(stderr, "[%s]get buffer memory fail.\n", __func__);
		free(buf);
		pclose(fp);
		return NULL;
	}
#ifdef __aarch64__
	int i=0;
	while(!feof(fp))
	{
		buf[i++] =fgetc(fp);
	}
	buf[i-1] = '\0';
#else
	c = -1;
	do{
		*(buf+(++c)) = fgetc(fp);
	}while(*(buf+c) != 0xFFFFFFFF);

	*(buf+c) = '\0';
#endif

	if(strlen(buf) == 0)
	{
		free(buf);
		retPointer = strcpy(ret, "");
		pclose(fp);
		return retPointer;
	}
	if(splitStr != NULL)
	{
		firstline = strtok(buf, "\n"); //only read the first line
		if(firstline != NULL)
		{
			pch = strtok(firstline, splitStr);
			if(index > 0)
			{
				for(c = 0; c < index; c++)
				{
					pch = strtok(NULL, splitStr);
				}
			}
			if(pch != NULL)
			{
				retPointer = strcpy(ret, pch);
			}
			else
			{
				retPointer = strcpy(ret, "");
			}
		}
	}
	else
	{
		retPointer = strcpy(ret, buf);
	}

	free(buf);
	if(retPointer != NULL)
	{
		retPointer = lib_SysInfo_TrimWhiteSpace(ret);
		pclose(fp);
		return retPointer;
	}
	else
	{
		pclose(fp);
		return NULL;
	}
}

char* MEMAllocting(const char* func)
{
	char* ret;

	ret = (char*)malloc(sizeof(char) * MAX_CMD_STR_LEN);

	if(ret == NULL)
	{
		fprintf(stderr, "[%s]Allocating memory fail.\n", func);
	}

	memset(ret, 0, MAX_CMD_STR_LEN);

	return ret;
}

char* MEMReallocting(char* inputStr)
{
	char* ret;
	
	if(inputStr != NULL)
	{
		ret = (char*)realloc(inputStr, strlen(inputStr) + 1);
	}
	else
	{
		ret = (char*)realloc(inputStr, 1);
	}

	if(ret != NULL)
	{
		inputStr = ret;
		*(inputStr+strlen(inputStr)) = '\0';
	}
	else
	{
		fprintf(stderr, "[%s]Reallocating memory fail.\n", __func__);
		free(inputStr);
		return NULL;
	}

	return inputStr;
}
#ifdef __aarch64__


char* GetJetsonResult(char* ret,char* item,int len){
	char *result;
	char *start_str = strstr(ret,item);

	char *end_str = strstr(start_str,",");
	if (end_str == NULL){
		end_str = strstr(start_str,"}");
	}
	int start_location = start_str -ret + len;
	int end_location = end_str-ret - 1;
	int ProductName_len = end_location - start_location;
	char *test;

	char *ProductName_str[ProductName_len];

	strncpy(ProductName_str,ret+start_location,ProductName_len);
	// printf("\nitem : %s , name: %s ",item ,ProductName_str);

	result = ProductName_str;
	return  result;
}

char* GetJetsonResult_int(char* ret,char* item,int len){
	char *result;
	char *start_str = strstr(ret,item);

	char *end_str = strstr(start_str,",");
	if (end_str == NULL){
		end_str = strstr(start_str,"}");
	}
	int start_location = start_str -ret + len;
	int end_location = end_str-ret;
	int ProductName_len = end_location - start_location;
	char *test;

	char *ProductName_str[ProductName_len];

	strncpy(ProductName_str,ret+start_location,ProductName_len);
	
	result = ProductName_str;
	
	return  result;
}
#endif

#ifdef WIN32

//#define WMIDEBUG

char** GetWMIData(char* resourcestring, char* querystring, int queryIndex, int numberofquery, ...)
{
	int i;
	int selectedIndex = -1;
	char** ret = (char**)malloc(sizeof(char*) * numberofquery);
	wchar_t* res_wchar = (wchar_t*)malloc(sizeof(wchar_t) * 200);
	wchar_t* query_wchar = (wchar_t*)malloc(sizeof(wchar_t) * 200);
	wchar_t* header_wchar = (wchar_t*)malloc(sizeof(wchar_t) * 200);

	swprintf(res_wchar, 200, L"%hs", resourcestring);
	swprintf(query_wchar, 200, L"%hs", querystring);
	//swprintf(header_wchar, 200, L"%hs", headerstring);

	// result code from COM calls
	HRESULT hr = 0;

	// COM interface pointers
	IWbemLocator         *locator  = NULL;
	IWbemServices        *services = NULL;
	IEnumWbemClassObject *results  = NULL;

	// BSTR strings we'll use (http://msdn.microsoft.com/en-us/library/ms221069.aspx)
	BSTR resource = SysAllocString(res_wchar);
	BSTR language = SysAllocString(L"WQL");
	BSTR query    = SysAllocString(query_wchar);

	// initialize COM
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	// connect to WMI
	hr = CoCreateInstance(&CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, &IID_IWbemLocator, (LPVOID *) &locator);
	hr = locator->lpVtbl->ConnectServer(locator, resource, NULL, NULL, NULL, 0, NULL, NULL, &services);

	// issue a WMI query
	hr = services->lpVtbl->ExecQuery(services, language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

#ifdef WMIDEBUG
	printf("Query string:%s\n", querystring);
#endif

	if(results != NULL) 
	{
		IWbemClassObject *result = NULL;
		ULONG returnedCount = 0;

		va_list vl;
		va_start(vl, numberofquery);

	 	// enumerate the retrieved objects
		while((hr = results->lpVtbl->Next(results, WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK) 
		{

#ifdef WMIDEBUG
			printf("selectedIndex:%d, queryIndex:%d\n", selectedIndex, queryIndex);
#endif
			if(++selectedIndex == queryIndex)
			{
				//printf("zzzzzzzzzzzzzzzz\n");
				for (i = 0; i < numberofquery; i++)
				{
#ifdef WMIDEBUG
					printf("Start allocat memory\n");
#endif
					char* tmp = (char*)malloc(sizeof(char) * 200);
#ifdef WMIDEBUG
					printf("End for allocated memory\n");
#endif
					swprintf(header_wchar, 200, L"%hs", va_arg(vl, char*));
#ifdef WMIDEBUG
					printf("swprintf\n");
#endif
					VARIANT var;
					// obtain the desired properties of the next result and print them out
					hr = result->lpVtbl->Get(result, header_wchar, 0, &var, 0, 0);

					if(hr == 0)
					{
#ifdef WMIDEBUG
						wprintf(L"%s, %d", header_wchar, var.vt);
#endif
						switch(var.vt)
						{
							case VT_NULL:
#ifdef WMIDEBUG
								wprintf(L"\n");
#endif
								sprintf(tmp, "");
								break;
							case VT_I4:
#ifdef WMIDEBUG
								wprintf(L", %d\n", var.bstrVal);
#endif
								sprintf(tmp, "%d", var.bstrVal);
								break;
							default:
#ifdef WMIDEBUG
								wprintf(L", %s\n", var.bstrVal);
#endif
								wcstombs(tmp, var.bstrVal, 200);
								break;
						}
					}
					/*if(var.vt == VT_BSTR)
					   wcstombs(tmp, var.bstrVal, 200);*/

					*(ret+i) = tmp;
					VariantClear(&var);
				}
				// release the current result object
				result->lpVtbl->Release(result);
				break;
			}
			result->lpVtbl->Release(result);
			//printf("Loop done.\n");
		}

#ifdef WMIDEBUG
		printf("root:%s, query string:%s\n", resourcestring, querystring);
#endif

		va_start(vl, numberofquery);

		for(i = 0; i < numberofquery; i++)
		{
#ifdef WMIDEBUG
			printf("Data column=%s, ", va_arg(vl, char*));
#endif
			if(selectedIndex != queryIndex)
			{
				char* tmp = (char*)malloc(sizeof(char) * 10);
				sprintf(tmp, "%d", 0);
				*(ret+i) = tmp;
			}
#ifdef WMIDEBUG
			printf("Get Value=%s, strlen=%d\n", *(ret+i), strlen(*(ret+i)));
#endif
		}

		va_end(vl);
	}


	// release WMI COM interfaces
	results->lpVtbl->Release(results);
	services->lpVtbl->Release(services);
	locator->lpVtbl->Release(locator);

	// unwind everything else we've allocated
	CoUninitialize();

	SysFreeString(query);
	SysFreeString(language);
	SysFreeString(resource);

	free(res_wchar);
	free(query_wchar);
	free(header_wchar);

	return ret;
}
#endif
