#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alstrings.h"

// construct linked list at once within an array
// return first
struct token_char_buffer * al_token_char_buffer_alloc(int times)
{
  struct token_char_buffer * buffers = calloc(times, sizeof(struct token_char_buffer));
  for (int i=0; i<times;i++)
    {
      buffers[i].first = buffers;
      // last will point on first circular.
      buffers[i].next = &buffers[(i+1)%times];
    }
  return buffers;
}
  
void al_token_char_buffer_init(struct token_char_buffer * buffer, int chars)
{
  buffer->bufsize = chars;
  buffer->bufpos = 0;
  buffer->buf = malloc (buffer->bufsize);
}

struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer, int length)
{
  struct token_char_buffer * next  = buffer->next;
  int bufsize = buffer->bufsize * 2;
  // last point on first, circular
  while ( ( next != NULL ) && ( next != buffer ) )
    {
      if ( next->buf == NULL )
	{
	  // grown
	  // at least length...
	  if ( bufsize < length )
	    {
	      bufsize = length;
	    }
	  next->bufsize = bufsize;
	  next->buf = calloc(1,bufsize);
	  return next;
	}
      else
	{
	  // found a place, no need to grow
	  if ( ( next->bufsize - next->bufpos ) >= length )
	    {
	      return next;
	    }
	}
      if (next->bufsize > 0 )
	{
	  bufsize = next->bufsize * 2;
	}
      next = next->next;
    }

  return next;
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
	  buffer = al_token_char_buffer_grow(buffer, data->length);
	}
      if ( buffer != NULL )
	{
	  char * buf = &buffer->buf[buffer->bufpos];
	  memcpy (buf, data->data.ptr, data->length);
	  buffer->bufpos += data->length;
	  return buf;
	}
      else
	{
	  fprintf(stderr,"[FATAL] token char buffer allocation shortage");
	  exit(1);

	}
    }
  else
    {
      fprintf(stderr,"[FATAL] token char buffer is NULL");
      exit(1);
    }
}
