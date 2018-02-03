#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alstrings.h"
#include "alcommon.h"

enum alstrings_debug_flags {
  ALSTRINGS_DEBUG_FLAG = 1,
};

static int alstrings_debug_flags = 1;

int alstrings_set_debug(int flags)
{
  int previous = alstrings_debug_flags;
  alstrings_debug_flags = flags;
  return previous;
}

int alstrings_debug_flag_is_set(int flag)
{
  return  ALC_FLAG_IS_SET(alstrings_debug_flags,flag);
}

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
  // can be freed with int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
  // don't set first or next  
}

// get last pointing on circular.
struct token_char_buffer *  al_token_char_buffer_get_previous(struct token_char_buffer * circular)
{
  struct token_char_buffer * buffer = circular;
  struct token_char_buffer * previous = circular;
  struct token_char_buffer * next = buffer->next;
  int max = 1000;
  while ( ( next != NULL ) && ( next != buffer ) && ( max > 0) )
    {      
      previous = next;
      next = next->next;
      --max;
    }
  if ( max <= 0 )
    {
      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
	{
	  fprintf(stderr,"[FATAL] long loop (infinite ? ) on  al_token_char_buffer_get_previous\n");
	}
      exit(1);
    }
  return previous;  
}

void al_token_char_buffer_rehead(struct token_char_buffer * newhead, struct token_char_buffer * circular)
{
  if ( ( newhead != NULL ) && ( circular != NULL ) && (newhead->next == NULL) )
    {
      // get last pointing on circular.
      struct token_char_buffer * previous =  al_token_char_buffer_get_previous(circular);
      // to make linus scream
      if ( previous != NULL)
	{
	  previous->next=newhead;
	}
      else
	{
	  circular->next=newhead;
	}
      newhead->next=circular;
    }
}

// pick from buffer one that can provide length , if not engough palce create a new one.
struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer, int length)
{
  struct token_char_buffer * next  = buffer->next;
  int bufsize = buffer->bufsize * 2;

  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      fprintf(stderr,"grow token_char_buffer %p %i/%i\n", buffer, buffer->bufpos, buffer->bufsize);
    }
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
	  next->bufpos=0;
	  next->bufsize = bufsize;
	  next->buf = calloc(1,bufsize);
	  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
	    {
	      fprintf(stderr,"allocate new next token_char_buffer %p %i/%i\n", next, next->bufpos, next->bufsize);
	    }
	  return next;
	}
      else
	{
	  // found a place, no need to grow
	  if ( ( next->bufsize - next->bufpos ) >= length )
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{		  
		  fprintf(stderr,"found place next token_char_buffer %p %i/%i\n", next, next->bufpos, next->bufsize);
		}
	      return next;
	    }
	}
      if (next->bufsize > 0 )
	{
	  bufsize = next->bufsize * 2;
	}
      next = next->next;
    }

  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      fprintf(stderr,"allocate buffer (%p) next (%p) !\n", buffer, next);
    }
  return next;
}

char * al_alloc_block(alstrings_ringbuffer_pointer * ringbufferp, int length)
{
  if ( ringbufferp != NULL )
    {
      struct token_char_buffer * buffer = (*ringbufferp);
      if ( length > 0 )
	{
	  if ( buffer != NULL )
	    {
	      if ((buffer->bufpos + length) >= buffer->bufsize)
		{
		  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		    {
		      fprintf (stderr,
			       "[WARNING] internal char buffer %p full (%i+%i)>=%i\n",
			       buffer, buffer->bufpos, length, buffer->bufsize);
		    }
		  buffer = al_token_char_buffer_grow(buffer, length);
		  if ( buffer != NULL )
		    {
		      // could consider this bucket as new head to not walk from start always
		      (*ringbufferp) = buffer;
		    }
		}
	      if ( buffer != NULL )
		{
		  char * buf = &buffer->buf[buffer->bufpos];
		  buffer->bufpos += length;
		  return buf;
		}
	      else
		{
		  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		    {
		      fprintf(stderr,"[FATAL] token char buffer allocation shortage");
		    }
		  exit(1);
		}
	    }
	  else
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{
		  fprintf(stderr,"[FATAL] token char buffer is NULL");
		}
	      exit(1);
	    }
	}
      else
	{
	  return NULL;
	}
    }
  return NULL;
}

char * al_copy_block(alstrings_ringbuffer_pointer * ringbufferp, struct alhash_datablock * data)
{
  if ( ringbufferp != NULL )
    {
      struct token_char_buffer * buffer = (*ringbufferp);
      if ( buffer != NULL )
	{
	  char * buf = al_alloc_block(ringbufferp, data->length);
	  if ( buf != NULL )
	    {
	      // if block ref NULL create a zeroed buffer.
	      if ( data->data.ptr != NULL )
		{
		  memcpy (buf, data->data.ptr, data->length);
		}
	      else
		{
		  bzero (buf, data->length);
		}
	      return buf;
	    }
	  else
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{
		  fprintf(stderr,"[FATAL] token char buffer allocation shortage");
		}
	      exit(1);
	    }
	}
    }
  return NULL;
}


void alstrings_ringbuffer_init_autogrow(alstrings_ringbuffer_pointer * ringbufferp, int buckets, int firstbucketlength)
{
  if ( ringbufferp != NULL )    
    {
      struct token_char_buffer * allocated = al_token_char_buffer_alloc(buckets);
      al_token_char_buffer_init(allocated,firstbucketlength);
      *ringbufferp = allocated;
     }
}

int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
{
  if ( bucket != NULL )
    {
      // see what is done in void al_token_char_buffer_init(struct token_char_buffer * buffer, int chars)
      if (bucket->buf != NULL )
	{
	  free( bucket->buf);
	}
      bucket->bufsize = 0;
      bucket->bufpos = 0;
    }
  // continue
  return 0;
}
			  
void alstrings_ringbuffer_walk_buckets(alstrings_ringbuffer_pointer ringbuffer, int (*callback) (alstrings_ringbuffer_pointer bucket, int count, void * data), void * data)
{
  struct token_char_buffer * buffer = ringbuffer;
  struct token_char_buffer * previous = NULL;  
  struct token_char_buffer * next = buffer->next;
  int count = 0;
  
  if ( callback(buffer,count,data) == 0)
    {
      while ( ( next != NULL ) && ( next != buffer ) )
	{
	  previous = next;
	  next = next->next;
	  ++ count;
	  if ( callback(previous,count,data) != 0 )
	    {
	      break;
	    }
	}
    }
}
  
void alstrings_ringbuffer_release(alstrings_ringbuffer_pointer * ringbufferp)
{
  if (ringbufferp != NULL )
    {
      struct token_char_buffer * torelease = (*ringbufferp);
      if ( torelease != NULL )
	{
	  alstrings_ringbuffer_walk_buckets(torelease, alstrings_freebucket, NULL);
	  if ( torelease->first != NULL)
	    {
	      free(torelease->first);
	    }
	  torelease->first = NULL;
	  (*ringbufferp) = (void *) 0xdeadbe07;
	}
    }
}
