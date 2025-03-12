#include "altoken.h"

#include <stdlib.h>
#include "aldebug_output.h"

// >=0 if add did work
int altoken_char_buffer_add_char(alstrings_ringbuffer_pointer ringbuffer, char c)
{
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  return alstrings_buffer_add_char(buffer,c,ALTOKEN_BUFSIZE_MIN);
}

void altoken_flush_char_buffer(alstrings_ringbuffer_pointer ringbuffer)
{
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  alstrings_buffer_flush(buffer);
}
