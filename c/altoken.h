#ifndef ALTOKEN_HEADER__
#define ALTOKEN_HEADER__

#include "alstrings.h"

#define ALTOKEN_BUFSIZE_MIN 256
#define ALTOKEN_BUFSIZE_WARNING 8192
#define ALTOKEN_BUFSIZE_MAX 128000

struct al_token {
  int token;
};

// add a char within token char buffer that will be flush at cut_string_object or flush_char_buffer
int altoken_char_buffer_add_char(struct token_char_buffer * ctx, char c);

void altoken_flush_char_buffer(struct token_char_buffer * ctx);

#endif
