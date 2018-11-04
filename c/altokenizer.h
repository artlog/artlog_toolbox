#ifndef ALTOKENIZER_HEADER__
#define ALTOKENIZER_HEADER__

#include "altoken.h"
#include "alstrings.h"

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
};

// intiialization will fully reset tokenizer content
void altokenizer_init(struct altokenizer * tokenizer);

// consume str and check all consumed chars string equals str content
void altokenizer_consume(struct altokenizer * tokenizer, void * data, char * str);

// mainly free allocated buffer for token buf, should not be shared.
void altokenizer_release(struct altokenizer * tokenizer);

#endif // ALTOKENIZER_HEADER__
