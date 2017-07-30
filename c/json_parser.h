#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

// everything for tokenizing.

#define JSON_PATH_MAX_CHARS 4096

// forward definitions
struct json_ctx;
struct json_object;

struct al_token {
  int token;
};

typedef char (*get_next_char)(struct json_ctx *ctx, void *data);
typedef void (*set_pushback_char)(struct json_ctx *ctx, void *data, char pushback);
// goal separate tokenizer and parsing.
typedef struct json_object* (*json_parse_func) (struct json_ctx *ctx, void *data, struct json_object * parent);
typedef struct al_token* (*al_tokenizer_func) (struct json_ctx *ctx, void *data);
typedef int (*add_token_char)(struct json_ctx *ctx, char token, char c);


/** keep position of line and column for a stream during a json parsing */
struct json_pos_info
{
  int line;
  int column;
};

/**
Level of char seen :
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

struct json_ctx
{
  struct json_level parenthesis; // parenthesis '(' ')' 
  struct json_level braket; // brakets '[' ']' 
  struct json_level dquote; // double quote '"'
  struct json_level squote; // simple quote "'"
  struct json_level variable; // variable "?" ; to use existing framework JSON_TOGGLE not really sound yet.
  get_next_char next_char;
  set_pushback_char pushback_char;
  json_parse_func unstack;
  al_tokenizer_func tokenizer;
  add_token_char add_char;
  char * buf;
  int debug_level;
  int bufpos;
  int bufsize;
  int pos;
  int internal_flags;
  // struct json_object * error; // null if no error encountered, else set to last error
  struct al_token last_token;
  struct json_pos_info pos_info;
};

#define JSON_OPEN(ctx,__member__,object)   ctx->__member__.open_level++;ctx-> __member__ .max_open_level++;object=new_ ## __member__(ctx);
#define JSON_CLOSE(ctx,__member__)   ctx->__member__.open_level--;ctx-> __member__.max_close_level++;
#define JSON_TOGGLE(ctx,__member__)   ctx->__member__.open_level++;ctx->__member__.max_open_level++; 

#define JSON_DEFINE_TOGGLE(__member__,__char__) \
  struct json_object * json_parse_ ## __member__ ## _level(struct json_ctx * ctx, void * data) \
{\
  int result = parse_until_escaped_level(ctx,data,__char__,'\\');\
  if ( result ) { \
    JSON_TOGGLE(ctx,__member__);			\
    return cut_string_object(ctx,__char__);		\
  }							\
  return (struct json_object *) NULL;		\
}\

#define JSON_TOKEN(token_name) \
  { ctx->last_token.token=JSON_TOKEN_ ##token_name ##_ID;\
    return &ctx->last_token; }

#define TOKEN_DEFINE_TOKENIZER(__token__,__char__) \
  struct al_token * tokenizer_ ## __token__ (struct json_ctx * ctx, void * data) \
{\
  int result = parse_until_escaped_level(ctx,data,__char__,'\\');\
  if ( result ) { \
    JSON_TOKEN(__token__);				\
  }							\
  return  NULL;		\
}\

/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context);

void memory_shortage(struct json_ctx * ctx);

/** set debug level 0 : none 
return previous debug flags
*/
int json_ctx_set_debug(struct json_ctx * ctx, int debug);

int json_context_get_debug(struct json_ctx *json_context);

void debug_tag(struct json_ctx *ctx,char c);

/**
 Actual parsing step

json_ctx current context ( will be updated )
void * data represent stream currently parsed, actual type depends on next_char function.
json_object parent of json object to be currently parsed.

return a json_concret'ized object representing full parsing for this level.
 **/
struct json_object * parse_level(struct json_ctx * ctx, void * data, struct json_object * parent);

/**
 used during parsing when a char should be re-parsed 
**/
void pushback_char(struct json_ctx *ctx, void *data, char pushback);

int add_char(struct json_ctx * ctx, char token, char c);

char next_char(struct json_ctx* ctx, void * data);

void flush_char_buffer(struct json_ctx * ctx);

// consume str and check all consumed chars string equals str content
int json_ctx_consume(struct json_ctx * ctx, void * data, char * str);

/**
 return internal parsing state. 9 means parsing did find a number.
 recognized pattern is added char by char to data using ctx->add_char()
*/
int parse_number_level(struct json_ctx * ctx, char first, void * data);

/**
 read until stop char that is eaten.
 if escape char is found then next char is added as it is 
return 1 of read up to unescaped char stop, else return 0.
**/
int parse_until_escaped_level(struct json_ctx * ctx, void * data, char stop, char escape);

#endif
