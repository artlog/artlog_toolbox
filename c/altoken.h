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
int altoken_char_buffer_add_char(alstrings_ringbuffer * ctx, char c);

void altoken_flush_char_buffer(alstrings_ringbuffer * ctx);

#endif
