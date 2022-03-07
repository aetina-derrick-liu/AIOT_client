#ifndef __JSONPARSER_HPP__
#define __JSONPARSER_HPP__

extern "C"
{
    #include "json-c/json.h"
}

struct json_object* get_json_object(json_object* rootObj, const char* key);

#endif

