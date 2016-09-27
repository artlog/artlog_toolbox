  
struct json_ctx;
struct json_object;

/* parameters for pretty printing */
struct print_ctx {
  int indent;
  int do_indent; // 0 no indent, >= 1 number of space by indent.
  char * s_indent;
};

void dump_ctx(struct json_ctx * ctx);

/** Where output is finaly done */
void dump_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

typedef char (*get_next_char)(struct json_ctx *ctx, void *data);
typedef void (*set_pushback_char)(struct json_ctx *ctx, void *data, char pushback);
typedef struct json_object* (*parse_func) (struct json_ctx *ctx, void *data, struct json_object * object);
typedef int (*add_token_char)(struct json_ctx *ctx, char token, char c);

struct json_pos_info {
  int line;
  int column;
};

/**
Level of char seen :
char depend on context 
parenthesis
braket
dquote
squte
**/
struct json_level {
  int open_level;
  int max_open_level; // level of  {,[,'," seen
  int max_close_level; // level of },],'," 'seen
};

/** a simple linked list of json_object */
struct json_link {
  struct json_object * value;
  struct json_link * next;
};

/** A growable is a json object TO BE
It will be reduced to a real json object at 'concrete' time
**/
struct json_growable {
  struct json_link head; // has a meaning ONLY if tail is non NULL.
  struct json_link * tail;
  int size;
  char final_type;
};

/** Parsing context of json */
struct json_ctx {
  struct json_level parenthesis;
  struct json_level braket;
  struct json_level dquote;
  struct json_level squote;  
  get_next_char next_char;
  set_pushback_char pushback_char;
  parse_func unstack;
  add_token_char add_char;
  char * buf;
  int bufpos;
  int bufsize;
  int pos;
  struct json_pos_info pos_info;
  struct json_object * root;
  struct json_object * tail;
};

/** a json_object that is a number */
struct json_number {
  int length;
  unsigned char * b;
};

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

struct json_object {
  char type;
  unsigned char pad[3];
  struct json_pos_info pos_info;
  union {
    struct json_number number;
    struct json_string string;
    struct json_list list;
    struct json_dict dict;
    struct json_pair pair;
    struct json_growable growable;
  }; 
};

void debug_tag(char c);

#define JSON_OPEN(ctx,__member__,object)   ctx->__member__.open_level++;ctx-> __member__ .max_open_level++;object=new_ ## __member__(ctx);
#define JSON_CLOSE(ctx,__member__)   ctx->__member__.open_level--;ctx-> __member__.max_close_level++;
#define JSON_TOGGLE(ctx,__member__)   ctx->__member__.open_level++;ctx->__member__.max_open_level++; 

void memory_shortage(struct json_ctx * ctx);

void syntax_error(struct json_ctx * ctx,void * data,struct json_object * object,struct json_object * parent);

// create a json string object from context buffer
struct json_object * cut_string_object(struct json_ctx * ctx, char objtype);

struct json_object * new_growable(struct json_ctx * ctx, char final_type);

#define JSON_DEFINE_NEW(__member__,__char__) struct json_object * new_ ## __member__(struct json_ctx * ctx) { return new_growable(ctx,__char__);};

#define JSON_DEFINE_TOGGLE(__member__,__char__) \
  struct json_object * parse_ ## __member__ ## _level(struct json_ctx * ctx, void * data, struct json_object * object) \
{\
  char c = ctx->next_char(ctx, data);\
  while ( c != 0)\
    {\
  if ( c == __char__)\
	{\
	  JSON_TOGGLE(ctx,__member__);\
	  return cut_string_object(ctx,__char__);\
	}\
  else if ( c == '\\' ) {				\
        c=ctx->next_char(ctx, data);\
	if ( c != 0 ) ctx->add_char(ctx,__char__, c);	\
  }\
  else {\
    ctx->add_char(ctx,__char__, c);		\
      }\
      c=ctx->next_char(ctx, data);\
    }\
  return (struct json_object *) NULL;		\
}\

/**
 Actual parsing step

json_ctx current context ( will be updated )
void * data represent stream currently parsed, actual type depends on next_char function.
json_object parent of json object to be currently parsed.
 **/
struct json_object * parse_level(struct json_ctx * ctx, void * data, struct json_object * parent);

void pushback_char(struct json_ctx *ctx, void *data, char pushback);

int add_char(struct json_ctx * ctx, char token, char c);

char next_char(struct json_ctx* ctx, void * data);
  
void dump_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void dump_ctx(struct json_ctx * ctx);

/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context);

/**
  return value with key keyname in object
**/
struct json_object * json_dict_get_value(char * keyname, struct json_object * object);

/**
  return value at index in list
**/
struct json_object * json_list_get( struct json_object * object, int index);
