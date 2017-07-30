#ifndef __JSON_HEADER__
#define __JSON_HEADER__

#include "json_parser.h"
#include "json_errors.h"

#define JSON_PATH_DEPTH 1024

/* parameters for pretty printing */
struct print_ctx
{
  int indent;
  int do_indent; // 0 no indent, >= 1 number of space by indent.
  char * s_indent;
};

/** print context to stderr */
void dump_ctx(struct json_ctx * ctx);

/** Where output is finaly done */
void dump_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

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

enum json_internal_flags {
  JSON_FLAG_IGNORE=1
};

/** Parsing context of json */

/** json_object that is a number is a json_string with a type '0' */

/* a json_object that is a string */
struct json_string {
  int length;
  char * chars;
};

/* a json list of nitems json objects */
struct json_list {
  int nitems;
  struct json_object * value[];
};

/* a json key:value */
struct json_pair {
  struct json_object * key; // should be json_string ?
  struct json_object * value;
};

/** dict : TODO should provide a hash access **/
struct json_dict {
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

struct json_path {
  char type; // '{' string is key of dict, '[' index is index of list , '*' automatic ( ie key or index is used depending on parsed structure )
  struct json_path * child;
  struct json_string string;
  int index;
};

struct json_object * syntax_error(struct json_ctx * ctx,enum json_syntax_error erroridx, void * data,struct json_object * object,struct json_object * parent);

// create a json string object from context buffer
struct json_object * cut_string_object(struct json_ctx * ctx, char objtype);

struct json_object * new_growable(struct json_ctx * ctx, char final_type);

#define JSON_DEFINE_NEW(__member__,__char__) struct json_object * new_ ## __member__(struct json_ctx * ctx) { return new_growable(ctx,__char__);};

  
void dump_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void dump_ctx(struct json_ctx * ctx);

/**
  return value with key keyname in object
**/
struct json_object * json_dict_get_value(char * keyname, struct json_object * object);

/**
  return value at index in list
**/
struct json_object * json_list_get( struct json_object * object, int index);

/** set debug level 0 : none 
return previous debug flags
*/
int json_set_debug(int debug);

/**
first lazzy implementation : compare them

return 1 if equals, 0 if different.
*/
int json_unify_object(
	       struct json_ctx * ctx, struct json_object * object,
	       struct json_ctx * other_ctx, struct json_object * other_object,
	       struct print_ctx * print_ctx);


void json_print_object_name(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

/**
given a json_path ex : menu.popup.menuitem.1 find the json object.
*/
struct json_object * json_walk_path(char * json_path, struct json_ctx * ctx, struct json_object * object);

/**
dump object in a flat view, ie using object name=value notation for every value.
 */
void json_flatten(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

enum json_walk_action {
  JSON_WALK_CONTINUE, // normal action, continue
  JSON_WALK_STOP, // stop at this step and return without error
  JSON_WALK_SKIP, // skip next
  JSON_WALK_NEXT_PARENT, // search within current parent list of dict is over, move to next
  JSON_WALK_FATAL_ERROR // internal problem : stop and return with error, avoid using any memory context, can be corrupted.
};
  
struct json_walk_leaf_callbacks  {
  /** specific data */
  void * data;
  struct json_ctx * ctx;
  struct print_ctx * print_ctx;
  /** notify a leaf ( ie a bare value, not a list or a dict ) */
  enum json_walk_action (*json_advertise_leaf) ( struct json_walk_leaf_callbacks * this, struct json_path * json_path, struct json_object * json_object);
  /** notify a variable ( part of template ) */
  enum json_walk_action (*json_advertise_set_variable) ( struct json_walk_leaf_callbacks * this, struct json_path * json_path, struct json_variable * json_variable);
  /** json path where to start search */
  struct json_path * root;
  /** json template for variables */
  struct json_object * template;
};

/**
  convert a json_object into an int if json object is already recognized as a number
  if object represent another number than an int, resultcan be whatever.
  else return 0
*/
int json_get_int(struct json_object * object );

char * json_get_string(struct json_object * object);

/** Where growables becomes real json objects **/
struct json_object * json_concrete(struct json_ctx * ctx, struct json_object * object);

struct json_object * parse_level_legacy(struct json_ctx * ctx, void * data, struct json_object * parent);

#endif
