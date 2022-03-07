#ifndef __JSONPARSER_H__
#define __JSONPARSER_H__

#include "json-c/json.h"

struct json_object* get_json_object(json_object* rootObj, const char* key);

#endif

