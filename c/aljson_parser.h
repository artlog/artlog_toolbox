#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "alstrings.h"

// everything for tokenizing.
// todo rename me as json_tokenizer.h
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
  JSON_TOKEN_OPEN_BRACKET_ID,
  JSON_TOKEN_CLOSE_BRACKET_ID,
  JSON_TOKEN_OPEN_BRACE_ID,
  JSON_TOKEN_CLOSE_BRACE_ID,
  JSON_TOKEN_COMMA_ID,
  JSON_TOKEN_COLON_ID,
  JSON_TOKEN_DQUOTE_ID,
  JSON_TOKEN_SQUOTE_ID,
  JSON_TOKEN_VARIABLE_ID,
  JSON_TOKEN_NUMBER_ID,
  JSON_TOKEN_TRUE_ID,
  JSON_TOKEN_FALSE_ID,
  JSON_TOKEN_NULL_ID,
  JSON_TOKEN_SEMI_COLON_ID,
  JSON_TOKEN_COMMENT_ID,
  JSON_TOKEN_QUESTION_ID,
  JSON_TOKEN_WORD_ID,
  JSON_TOKEN_AMPERSAND_ID,
  JSON_TOKEN_LOGICAL_AND_ID,
  JSON_TOKEN_PIPE_ID,
  JSON_TOKEN_LOGICAL_OR_ID,
  JSON_TOKEN_DOT_ID,
  JSON_TOKEN_STAR_ID,
  JSON_TOKEN_EQUAL_ID,
  JSON_TOKEN_COMPARE_EQUAL_ID,
  JSON_TOKEN_SUPERIOR_ID,
  JSON_TOKEN_SUPERIOR_EQUAL_ID,
  JSON_TOKEN_INFERIOR_ID,
  JSON_TOKEN_INFERIOR_EQUAL_ID,
  JSON_TOKEN_PERCENT_ID,
  JSON_TOKEN_PRAGMA_ID,
  JSON_TOKEN_EXCLAMATION_ID,
  JSON_TOKEN_COMPARE_DIFFERENT_ID,
  JSON_TOKEN_MINUS_ID, // -  hyphen
  JSON_TOKEN_RIGHT_ARROW_ID, // ->
  JSON_TOKEN_PLUS_ID,
  JSON_TOKEN_INCREMENT_ID, // ++
  JSON_TOKEN_ADD_ID, // +=
  JSON_TOKEN_DECREMENT_ID, // --
  JSON_TOKEN_SUBTRACT_ID, // -= yes subtract and not substract which is frenglish.
  JSON_TOKEN_PUSHBACK_ID
};

enum tokenizer_debug_flag {
  TOKENIZER_DEBUG_0 = 1,
  TOKENIZER_DEBUG_ADD = 32,
  TOKENIZER_DEBUG_PUSHBACK = 32
};

enum json_internal_flags {
  JSON_FLAG_IGNORE=1
};

// invalid typedef char (*)(struct json_ctx *ctx, void *data) get_next_char;
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
  int debug_level;
  // add a char to currently parsed token.
  add_token_char add_char;
  // for add_char usage
  struct token_char_buffer token_buf;
  // byte position within 'possible' input stream
  int pos;
  int internal_flags;
  struct al_token last_token;
  struct json_pos_info pos_info;
};


#define JSON_TOKEN(token_name) \
  { ctx->last_token.token=JSON_TOKEN_ ##token_name ##_ID;\
    return &ctx->last_token; }

#define TOKEN_DECLARE_TOKENIZER(__token__,__char__) \
  struct al_token * tokenizer_ ## __token__ (struct json_ctx * ctx, void * data)

#define TOKEN_DEFINE_TOKENIZER(__token__,__char__) \
  TOKEN_DECLARE_TOKENIZER(__token__,__char__) \
{\
  int result = parse_until_escaped_level(ctx,data,__char__,'\\');\
  if ( result ) { \
    JSON_TOKEN(__token__);				\
  }							\
  return  NULL;		\
}\

/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context, get_next_char next_char);

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

void flush_char_buffer(struct token_char_buffer * token_char_buffer);

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


struct al_token * tokenizer_NUMBER(struct json_ctx * ctx, char first, void * data);

#endif
