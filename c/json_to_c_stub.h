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

/* ===== READING JSON === */

#define AL_JSON_IS_DICT(json_object) (json_object->type == '{')

#define AL_JSON_IS_LIST(json_object) (json_object->type == '[')

// to complete, to check + ref info
#define AL_JSON_IS_REF(json_object) (json_object->type == '"')

#define AL_JSON_IS_NULL_CONST(json_object) (json_object->type == 'n')

// 'unused' because arguments should match with AL_GET_BY_ORDER ( AL_GET ##kind )
#define AL_GET_WITH_NAME(a,json_object,unused) json_dict_get_value(#a, json_object)

#define AL_GET_BY_ORDER(a,json_object,index) json_list_get(json_object,index)

#define AL_GET_JSON_INT(test,a,json_object,index,kind)			\
  { struct json_object * local_json_ ##a = AL_GET_ ##kind(a, json_object,index);\
    if ( local_json_ ##a != NULL ) {\
      test->a = json_get_int(local_json_ ##a);}\
  }

#define AL_GET_JSON_INT_WITH_NAME(test,a,json_object) AL_GET_JSON_INT(test,a,json_object,0,WITH_NAME)

#define AL_GET_JSON_INT_BY_ORDER(test,a,json_object,index) AL_GET_JSON_INT(test,a,json_object,index,BY_ORDER)
  
#define AL_GET_JSON_STRING(test,a,json_object,index,kind)			\
  { struct json_object * local_json_ ##a = AL_GET_ ##kind(a, json_object,index); \
    test->a = json_get_cstring(local_json_ ##a); }

#define AL_GET_JSON_STRING_WITH_NAME(test,a,json_object) AL_GET_JSON_STRING(test,a,json_object,0,WITH_NAME)

#define AL_GET_JSON_STRING_BY_ORDER(test,a,json_object,index)  AL_GET_JSON_STRING(test,a,json_object,index,BY_ORDER)

#define AL_GET_JSON_STRUCT_POINTER(struct_name, test,d,json_object,index, kind) \
  struct json_object * local_json_ ##d = AL_GET_ ##kind(d,json_object,index);		\
  if ( local_json_ ##d != NULL ) \
    {\
      if ( AL_JSON_IS_NULL_CONST(local_json_ ##d) ) {  test->d = NULL;}	\
      else {								\
	  test->d=malloc(sizeof(*test->d)); \
	  if ( test->d != NULL ) { json_c_ ##struct_name ##_from_json_auto(test->d,local_json_ ##d);} \
	}\
    }

#define AL_GET_JSON_NEW_STRUCT(struct_name, test,d)			\
  {									\
    test->d=malloc(sizeof(*test->d));					\
    if ( test->d != NULL ) { json_c_ ##struct_name ##_from_json_auto(test->d,local_json_ ##d);} \
  }

#define AL_GET_JSON_STRUCT_POINTER_WITH_NAME(struct_name, test,d,json_object) \
  struct json_object * local_json_ ##d = json_dict_get_value("&" #d, json_object); \
  if ( local_json_ ##d != NULL ) \
    {\
      if ( AL_JSON_IS_NULL_CONST(local_json_ ##d) ) {  test->d = NULL;}	\
      else { \
	if ( AL_JSON_IS_REF(local_json_ ##d) ) {   local_json_ ##d = json_to_c_stub_get_ref(local_json_ ##d, json_object); } \
          AL_GET_JSON_NEW_STRUCT(struct_name,test,d)		\
	  }							\
   }

#define AL_GET_JSON_STRUCT(struct_name, test,d,json_object,index, kind) \
  struct json_object * local_json_ ##d = AL_GET_ ##kind(d,json_object,index); \
  if ( ! AL_JSON_IS_NULL_CONST(local_json_ ##d) )			\
      { \
	json_c_ ##struct_name ##_from_json_auto(&test->d,local_json_ ##d); \
      }


/* ========= WRITING JSON =========== */

/**
 construct a new json pair named by key 'name' and with json_object value 'value'
 return a json pair type 
*/
struct json_object * json_c_add_json_object_member
       ( char * name,
	 struct json_object * value,
	 struct json_parser_ctx * ctx,
	 struct token_char_buffer * allocator );


// json_object pair type.
struct json_object * json_c_add_int_member
       ( char * name,
	 int value,
	 struct json_parser_ctx * ctx,
	 struct token_char_buffer * allocator );


// json_object pair type.
// capture char value content.
struct json_object * json_c_add_string_member
       ( char * name,
	 char * value,
	 struct json_parser_ctx * ctx,
	 struct token_char_buffer * allocator );

struct json_object * json_to_c_stub_get_ref( struct json_object * json_ref, struct json_object * json_root);

#endif
