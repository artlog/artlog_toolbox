#ifndef  _JSON_TO_C_STUB_H_
#define  _JSON_TO_C_STUB_H_

#include "aljson.h"


/**

allows to quickly map a json structure into a C structure 

**/


/**
by name :
{"a":1,"b":2,"c":3,d:"4"}

by order :
[1,2,3]
*/

#define AL_JSON_IS_DICT(json_object) (json_object->type == '{')

#define AL_JSON_IS_LIST(json_object) (json_object->type == '[')

#define AL_JSON_IS_NULL_CONST(json_object) (json_object->type == 'n')

#define AL_GET_WITH_NAME(a,json_object,index) json_dict_get_value(#a, json_object)

#define AL_GET_BY_ORDER(a,json_object,index) json_list_get(json_object,index)

#define AL_GET_JSON_INT(test,a,json_object,index,kind)			\
  { struct json_object * local_json_ ##a = AL_GET_ ##kind(a, json_object,index); \
    test->a = json_get_int(local_json_ ##a); }

#define AL_GET_JSON_INT_WITH_NAME(test,a,json_object) AL_GET_JSON_INT(test,a,json_object,0,WITH_NAME)

#define AL_GET_JSON_INT_BY_ORDER(test,a,json_object,index) AL_GET_JSON_INT(test,a,json_object,index,BY_ORDER)
  
#define AL_GET_JSON_STRING(test,a,json_object,index,kind)			\
  { struct json_object * local_json_ ##a = AL_GET_ ##kind(a, json_object,index); \
    test->a = json_get_string(local_json_ ##a); }

#define AL_GET_JSON_STRING_WITH_NAME(test,a,json_object) AL_GET_JSON_STRING(test,a,json_object,0,WITH_NAME)

#define AL_GET_JSON_STRING_BY_ORDER(test,a,json_object,index)  AL_GET_JSON_STRING(test,a,json_object,index,BY_ORDER)

#define AL_GET_JSON_STRUCT_POINTER(struct_name, test,d,json_object,index, kind) \
  struct json_object * local_json_ ##d = AL_GET_ ##kind(d,json_object,index);		\
  if ( AL_JSON_IS_NULL_CONST(local_json_ ##d) ) {  test->d = NULL;}	\
      else { \
	  test->d=malloc(sizeof(*test->d)); \
	  if ( test->d != NULL ) { json_c_ ##struct_name ##_from_json_auto(test->d,local_json_ ##d);} \
	}

#define AL_GET_JSON_STRUCT(struct_name, test,d,json_object,index, kind) \
  struct json_object * local_json_ ##d = AL_GET_ ##kind(d,json_object,index); \
  if ( ! AL_JSON_IS_NULL_CONST(local_json_ ##d) )			\
      { \
	json_c_ ##struct_name ##_from_json_auto(&test->d,local_json_ ##d); \
      }


#endif
