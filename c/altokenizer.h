#ifndef ALTOKENIZER_HEADER__
#define ALTOKENIZER_HEADER__

#include "altoken.h"
#include "alstrings.h"
#include "alhash.h"

struct altokenizer;

typedef char (*altokenizer_get_next_char)(struct altokenizer *ctx, void *data);
typedef void (*altokenizer_set_pushback_char)(struct altokenizer *ctx, void *data, char pushback);
typedef struct al_token* (*altokenizer_func) (struct altokenizer *ctx, void *data);
typedef int (*altokenizer_add_token_char)(struct altokenizer *ctx, char token, char c);

struct altokenizer
{
  ALDEBUG_DEFINE_FLAG(debug_level)
  altokenizer_get_next_char next_char;
  altokenizer_set_pushback_char pushback_char;
  altokenizer_func tokenizer;
  // add a char to currently parsed token.
  altokenizer_add_token_char add_char;
  // for add_char usage, created and grown by add_char
  struct token_char_buffer token_buf;
  struct al_token last_token;
  alhash_context context;
  int words;
};


#define ALTOKENIZER_TOKEN(token_name) \
  { ctx->last_token.token=ALTOKENIZER_TOKEN_ ##token_name ##_ID;\
    return &ctx->last_token; }

#define ALTOKENIZER_DECLARE_TOKENIZER(__token__)		\
  struct al_token * altokenizer_ ## __token__ (struct altokenizer * ctx, void * data)

// intiialization will fully reset tokenizer content
void altokenizer_init(struct altokenizer * tokenizer);

// consume str and check all consumed chars string equals str content
int altokenizer_consume(struct altokenizer * tokenizer, void * data, char * str);

// add char for a token todo remove useless parameter token
void altokenizer_add_char(struct altokenizer *tokenizer, char token, char c);

// return a new token content
// key is word value is token ( todo currently value is key too ).
struct alhash_entry * altokenizer_make_token(struct altokenizer *tokenizer, struct al_token * token, char c);

// mainly free allocated buffer for token buf, should not be shared.
void altokenizer_release(struct altokenizer * tokenizer);

// number of characters pending to be cut
int altokenizer_get_pending_chars(struct altokenizer * tokenizer);

  // reset when word is parsed and recognized as either a reserved word or stored in variable dict with cut_string.
void altokenizer_reset_buffer_pos (struct altokenizer *tokenizer);


#endif // ALTOKENIZER_HEADER__
