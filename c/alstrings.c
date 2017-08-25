#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alstrings.h"
#include "todo.h"

struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer)
{
  todo("[FATAL] growing token char buffer not implemented");
  exit(1);
  return NULL;
}

char * al_copy_block(struct token_char_buffer * buffer, struct alhash_datablock * data)
{
  if ( buffer != NULL )
    {      
      if ((buffer->bufpos + data->length) >= buffer->bufsize)
	{
	  fprintf (stderr,
		   "[WARNING] internal char buffer for words full %i+%i>=%i",
		   buffer->bufpos, data->length, buffer->bufsize);
	  al_token_char_buffer_grow(buffer);
	}
      char * buf = &buffer->buf[buffer->bufpos];
      memcpy (buf, data->data, data->length);
      buffer->bufpos += data->length;
      return buf;
    }
  else
    {
      todo("[FATAL] token char buffer is NULL");
      exit(1);
    }
}
