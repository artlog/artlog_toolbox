#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

// everything for tokenizing.
// does not know json_object at all.

#define JSON_PATH_MAX_CHARS 4096

// forward definitions
struct json_ctx;

struct al_token {
  int token;
};

enum json_token_id {
  JSON_TOKEN_EOF_ID,
  JSON_TOKEN_OPEN_PARENTHESIS_ID,
  JSON_TOKEN_CLOSE_PARENTHESIS_ID,
  JSON_TOKEN_OPEN_BRAKET_ID,
  JSON_TOKEN_CLOSE_BRAKET_ID,
  JSON_TOKEN_COMA_ID,
  JSON_TOKEN_DOUBLE_POINT_ID,
  JSON_TOKEN_DQUOTE_ID,
  JSON_TOKEN_SQUOTE_ID,
  JSON_TOKEN_VARIABLE_ID,
  JSON_TOKEN_NUMBER_ID,
  JSON_TOKEN_TRUE_ID,
  JSON_TOKEN_FALSE_ID,
  JSON_TOKEN_NULL_ID
};

enum json_internal_flags {
  JSON_FLAG_IGNORE=1
};

typedef char (*get_next_char)(struct json_ctx *ctx, void *data);
typedef void (*set_pushback_char)(struct json_ctx *ctx, void *data, char pushback);
typedef struct al_token* (*al_tokenizer_func) (struct json_ctx *ctx, void *data);
typedef int (*add_token_char)(struct json_ctx *ctx, char token, char c);


/** keep position of line and column for a stream during a json parsing */
struct json_pos_info
{
  int line;
  int column;
};

struct json_ctx
{
  get_next_char next_char;
  set_pushback_char pushback_char;
  al_tokenizer_func tokenizer;
  add_token_char add_char;
  char * buf;
  int debug_level;
  int bufpos;
  int bufsize;
  int pos;
  int internal_flags;
  struct al_token last_token;
  struct json_pos_info pos_info;
};


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

struct al_token * json_tokenizer(struct json_ctx * ctx, void * data);


#endif
