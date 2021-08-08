#include "altoken.h"

#include <stdlib.h>
#include "aldebug_output.h"

// PERHAPS full altoken could end in alstring ?

int altoken_char_buffer_add_char(alstrings_ringbuffer_pointer ringbuffer, char c)
{
  int bufsize=ALTOKEN_BUFSIZE_MIN;
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  if (buffer->buf == NULL)
    {
      buffer->buf=calloc(1,bufsize);
      //buffer->buf[bufsize-1]=0;
      buffer->bufpos=0;
      buffer->bufsize=bufsize;
    }
  if (buffer->bufpos+1>=buffer->bufsize)
    {
      bufsize=buffer->bufsize + buffer->bufsize / 2;
      if ( bufsize > ALTOKEN_BUFSIZE_MAX )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_MAX);
	  exit(0);
	}
      if ( bufsize > ALTOKEN_BUFSIZE_WARNING )
	{
	  aldebug_printf(DBGSTREAM,"[WARNING] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_WARNING);
	}
      char * newbuf=realloc(buffer->buf,bufsize);
      if (newbuf != NULL)
	{
	  //done by realloc
	  //memcpy(newbuf,buffer->buf,buffer->bufsize);
	  //free(buffer->buf);
	  buffer->buf[bufsize-1]=0;
	  buffer->bufsize=bufsize; 
	  buffer->buf=newbuf;
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"FATAL memory shortage in %s %s %i\n", __FILE__, __FUNCTION__, __LINE__ );
	}
    }
  buffer->buf[buffer->bufpos++]=c;
  return 0;
}

void altoken_flush_char_buffer(alstrings_ringbuffer_pointer ringbuffer)
{
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  if (buffer->buf != NULL )
    {
      free(buffer->buf);
      bzero(buffer, sizeof(*buffer));
    }
}
