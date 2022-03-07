#include "JsonParser.hpp"

struct json_object* get_json_object(json_object* rootObj, const char* key)
{
	struct json_object* returnObj;
	if(json_object_object_get_ex(rootObj, key, &returnObj))
	{
		return returnObj;
	}
	return NULL;
}

