#ifndef __ALJSON_HEADER__
#define __ALJSON_HEADER__

#include "aljson_parser.h"
#include "aljson_errors.h"
#include "alhash.h"

/**
Level of token seen :
char depend on context ( see struct json_ctx )
parenthesis
braket
dquote
squte
**/
struct json_level
{
  int open_level;
  int max_open_level; // level of  {,[,'," seen
  int max_close_level; // level of },],'," 'seen
};


struct json_parser_ctx
{
  struct alparser_ctx alparser;
  
  struct json_level parenthesis; // parenthesis '(' ')' 
  struct json_level braket; // brakets '[' ']' 
  struct json_level dquote; // double quote '"'
  struct json_level squote; // simple quote "'"
  struct json_level variable; // variable "?" ; to use existing framework JSON_TOGGLE not really sound yet.
  struct json_ctx * tokenizer;
  int parsing_depth; // stack calll on recursive parsing.
  int max_depth; // protect stack calls.
};

struct json_object;


/** a simple linked list of json_object */
struct json_link
{
  struct json_object * value;
  struct json_link * next;
};

/** A growable is a json object TO BE
It will be reduced to a real json object at 'concrete' time
when number of link component will be wellknown.
**/
struct json_growable
{
  struct json_link head; // has a meaning ONLY if tail is non NULL.
  struct json_link * tail;
  int size;
  char final_type;
};

/** Parsing context of json */

/** json_object that is a number is a json_string with a type '0' */

/* a json_object that is a string */
struct json_string {
  struct alhash_datablock internal;
};

/* a json list of nitems json objects */
struct json_list {
  void * dummy;
  int nitems;
  struct json_object * value[];
};

/* a json key:value */
struct json_pair {
  struct json_object * key; // should be json_string ?
  struct json_object * value;
};

/** dict and provide a hash access **/
struct json_dict {
  // contains hashtable dict, localcontext.dict.context can be NULL if unset
  struct alparser_ctx localcontext;
  int nitems;
  struct json_pair * items[];
};

/**
  different from a pair since it is a template filled by unification
*/
struct json_variable {
  struct json_object * key; // should be json_string ?
  struct json_object * value;
  int bound;
};

// todo , subset of TOKEN_FALSE ... 
enum json_internal_constant {
  JSON_CONSTANT_TRUE=0,
  JSON_CONSTANT_FALSE,
  JSON_CONSTANT_NULL,
  JSON_CONSTANT_LAST
};

struct json_constant {
  enum json_internal_constant value;
};

struct json_info_error {
  struct json_string string;
  struct json_pos_info where;
  enum json_syntax_error erroridx;
};
  
/** any kind of json object */
struct json_object {
  char type; // use to select real object type within union
  unsigned char pad[3];
  int shares; // number of times it is referenced a json_variable
  struct json_pos_info pos_info;
  struct json_object * owner; // parent owner used to construct naming.
  int index; // index in parent if a list
  void * private_data; // can be used to attach to a json_object some internal data.
  union {
    struct json_constant * constant; // 't' true 'f' false 'n' null
    // struct json_number number; // '0' in fact this is string with type '0'
    struct json_string string;  // '"' '\' or '$' ( internaly used to name variables ) or '0' for number
    struct json_list list; // '['
    struct json_dict dict; // '{'
    struct json_pair pair; // ':'
    struct json_growable growable; // 'G'
    struct json_variable variable; // '?'
    struct json_info_error error; // 'E'
  }; 
};

/** Where output is finaly done  **/

struct json_object * syntax_error(struct json_parser_ctx * ctx,enum json_syntax_error erroridx, void * data,struct json_object * object,struct json_object * parent);

// create a json object allocated from allocator
// if data is non NULL it is copied as json_object.string.internal.
struct json_object * aljson_new_json_object(char objtype, alstrings_ringbuffer_pointer * allocator, struct alhash_datablock  * data);

/* internal only 
struct json_object * aljson_new_json_string(struct json_ctx * ctx, char objtype, struct alhash_datablock  * data);
*/
 
struct json_object * aljson_new_pair_key(struct json_parser_ctx * parser, struct json_object * key);
  
struct json_object * aljson_new_growable(struct json_parser_ctx * ctx, char final_type);

#define JSON_DEFINE_NEW(__member__,__char__) struct json_object * aljson_new_ ## __member__(struct json_parser_ctx * ctx) { return aljson_new_growable(ctx,__char__);};

  
void dump_ctx(struct json_parser_ctx * ctx);

/**
  return value with key keyname in object
**/
struct json_object * json_dict_get_value(const char * keyname, struct json_object * object);

/**
 for each element of json_dict in definition order
 if callback returns NULL it continues, else it stops and return what callback returned
 **/
void * aljson_dict_foreach(
			 struct json_object * object,
			 void * (* callback) (struct json_object * key, struct json_object * value, void * data),
			 void * data);

/**
  return value at index in list
**/
struct json_object * json_list_get( struct json_object * object, int index);

/** set debug level 0 : none 
return previous debug flags
*/
int json_set_debug(int debug);


/**
  convert a json_object into an int if json object is already recognized as a number
  if object represent another number than an int, resultcan be whatever.
  else return 0
*/
int json_get_int(struct json_object * object );

float json_get_float(struct json_object * object );

char * json_get_string(struct json_object * object);

/** make sure to get a c string, allocate it if needed ...*/
char * json_get_cstring(struct json_object * object);

/** add a json object to a growable */
void aljson_add_to_growable(struct json_parser_ctx * ctx,struct json_growable * growable,struct json_object * object);

/** Where growables becomes real json objects **/
struct json_object * aljson_concrete(struct json_parser_ctx * ctx, struct json_object * object);

/**
 Actual parsing step

json_ctx current context ( will be updated )
void * data represent stream currently parsed, actual type depends on next_char function.
json_object parent of json object to be currently parsed.

return a json_concret'ized object representing full parsing for this level.
 **/
struct json_object * parse_level(struct json_parser_ctx * ctx, void * data, struct json_object * parent);

struct json_object * cut_string_object(struct json_parser_ctx * ctx, char objtype);

#define JSON_OPEN(ctx,__member__,object)   ctx->__member__.open_level++;ctx-> __member__ .max_open_level++;object=aljson_new_ ## __member__(ctx);
#define JSON_CLOSE(ctx,__member__)   ctx->__member__.open_level--;ctx-> __member__.max_close_level++;
#define JSON_TOGGLE(ctx,__member__)   ctx->__member__.open_level++;ctx->__member__.max_open_level++; 

#define JSON_DEFINE_TOGGLE(__member__,__char__) \
  struct json_object * json_parse_ ## __member__ ## _level(struct json_parser_ctx * ctx, void * data) \
{\
  int result = parse_until_escaped_level(ctx->tokenizer,data,__char__,'\\');\
  if ( result ) { \
    JSON_TOGGLE(ctx,__member__);			\
    return cut_string_object(ctx,__char__);		\
  }							\
  return (struct json_object *) NULL;		\
}\

// goal separate tokenizer and parsing.
typedef struct json_object* (*json_parse_func) (struct json_parser_ctx *ctx, void *data, struct json_object * parent);

// kindnyi for further use, specify type of data attach, allow to attach mulitple kind of data.
// return 1 if attach worked.
int aljson_attach_private_data(void * kindnyi, struct json_object * json, void * data);


#endif
