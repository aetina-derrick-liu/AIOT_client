#ifndef __JSONDEBUGGER_HPP__
#define __JSONDEBUGGER_HPP__

struct json_object* jsondebug_newobj(const char *file, int line, const char *func);
struct json_object* jsondebug_newarray(const char *file, int line, const char *func);
void jsondebug_addobj(struct json_object*	obj, const char*	key, struct json_object* val,
                      const char *file, int line, const char *func);
int jsondebug_addarray(struct json_object* obj,struct json_object* val,
                       const char *file, int line, const char *func);
void jsondebug_putobj(struct json_object*	obj	, const char *file, int line, const char *func);

#define json_object_new_object() jsondebug_newobj(__FILE__, __LINE__, __FUNCTION__)
#define json_object_new_array() jsondebug_newarray(__FILE__, __LINE__, __FUNCTION__)
#define json_object_object_add(X, Y, Z) jsondebug_addobj(X, Y, Z, __FILE__, __LINE__, __FUNCTION__)
#define json_object_array_add(X, Y) jsondebug_addarray(X,Y, __FILE__, __LINE__, __FUNCTION__)
#define json_object_put(X) jsondebug_putobj(X, __FILE__, __LINE__, __FUNCTION__)

#endif