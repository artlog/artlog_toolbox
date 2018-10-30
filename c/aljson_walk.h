#ifndef ALJSON_WALK_HEADER__
#define ALJSON_WALK_HEADER__

#include "aljson.h"
#include "aljson_print.h"

struct json_path {
  char type; // '{' string is key of dict, '[' index is index of list , '*' automatic ( ie key or index is used depending on parsed structure )
  struct json_path * child;
  struct json_string string;
  int index;
};

/**
given a json_path ex : menu.popup.menuitem.1 find the json object.
*/
struct json_object * aljson_walk_path(char * json_path, struct json_parser_ctx * ctx, struct json_object * object);


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
  struct json_parser_ctx * ctx;
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

#endif
