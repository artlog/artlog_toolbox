#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alstrings.h"
#include "alcommon.h"
#include "aldebug.h"

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
  
void al_token_char_buffer_init_internal(alstrings_ringbuffer_pointer buffer, int chars)
{

  buffer->bufsize = chars;
  buffer->bufpos = 0;
  buffer->buf = malloc (buffer->bufsize);
  // can be freed with int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
  // don't set first or next  
}

void al_token_char_buffer_init(alstrings_ringbuffer_pointer buffer, int chars)
{
  aldebug_printf(NULL,"[WARNING] use alstrings_ringbuffer_init_autogrow instead of deprecated  al_token_char_buffer_init\n");  
  al_token_char_buffer_init_internal(buffer,chars);
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
	  aldebug_printf(NULL,"[FATAL] long loop (infinite ? ) on  al_token_char_buffer_get_previous\n");
	}
      exit(1);
    }
  return previous;  
}

/* insert head to a circular buffer */
// DEPRECATED removed from api
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

// pick from buffer one that can provide length , if not enough place create a new one.
struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer, int length)
{
  struct token_char_buffer * next  = buffer->next;
  int bufsize = buffer->bufsize * 2;

  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      aldebug_printf(NULL,"grow token_char_buffer %p %i/%i\n", buffer, buffer->bufpos, buffer->bufsize);
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
	      aldebug_printf(NULL,"allocate new next token_char_buffer %p %i/%i\n", next, next->bufpos, next->bufsize);
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
		  aldebug_printf(NULL,"found place next token_char_buffer %p %i/%i\n", next, next->bufpos, next->bufsize);
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

  // what to conclude here if next == buffer ?
  // ==> that it was not possible to allocate a buffer
  if (  next == buffer )
    {
      aldebug_printf(NULL,"[WARNING] not possible to allocate a buffer. allocate buffer (%p) next (%p) \n", buffer, next);
      // shouldn't we return NULL ?
    }
  
  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      aldebug_printf(NULL,"allocate buffer (%p) next (%p) !\n", buffer, next);
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
		      aldebug_printf(NULL,"[FATAL] token char buffer allocation shortage");
		    }
		  exit(1);
		}
	    }
	  else
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{
		  aldebug_printf(NULL,"[FATAL] token char buffer is NULL");
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

char * al_copy_block(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * data)
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
		  aldebug_printf(NULL,"[FATAL] token char buffer allocation shortage");
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
      al_token_char_buffer_init_internal(allocated,firstbucketlength);
      *ringbufferp = allocated;
     }
}

int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
{
  if ( bucket != NULL )
    {
      // see what is done in void al_token_char_buffer_init_internal(struct token_char_buffer * buffer, int chars)
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

void aldatablock_bzero(aldatablock * data,int offset, int length)
{
  // todo check datablock type
  if ( ( offset >=0 ) && ( data->length >= offset + length ) )
  {
    bzero(&data->data.charptr[offset],length);
  }
}

// return new offset 
int aldatablock_write_uint64be(aldatablock * data, int offset, unsigned long long value )
{
  // todo check datablock type
  if ( data->length >= offset + 8 )
    {
      char * intern = &data->data.charptr[offset];
      char * hack = (char *) &value;
      // if LittleEndian ask gcc
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      intern[0]=hack[7];
      intern[1]=hack[6];
      intern[2]=hack[5];
      intern[3]=hack[4];
      intern[4]=hack[3];
      intern[5]=hack[2];
      intern[6]=hack[1];
      intern[7]=hack[0];      
#else
      memcpy(intern,hack,8);
#endif

    }     
	
  return offset + 8;
}

int aldatablock_write_byte(aldatablock * data, int offset, unsigned char value)
{
  // todo check datablock type
  if ( data->length > offset )
    {
      data->data.ucharptr[offset] = value;
    }
  return offset + 1;
}

// read an unsigned int that was stored in big endian at offset in datablock
unsigned int aldatablock_get_uint32be(aldatablock * data, int offset)
{
  // todo check datablock type
  unsigned int result = 0;
  if ( data->length >= offset + 4)
    {
      unsigned char * intern = (unsigned char *) &result;
      unsigned char * hack = &data->data.ucharptr[offset];
  
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      intern[0]=hack[3];
      intern[1]=hack[2];
      intern[2]=hack[1];
      intern[3]=hack[0];
#else
      memcpy(intern,hack,8);
#endif
    }
  else
    {
      aldebug_printf(NULL,"get int out of bound %i/%i\n", offset,data->length);
    }
  return result;  
}
  

void aldatablock_dump( aldatablock * block )
{
  if ( block != NULL )
    {
      if ( block->data.ptr != NULL )
	{
	  for (int i=0; i < block->length / sizeof(int); i ++)
	    {
	      aldebug_printf(NULL,"%08x ", block->data.uintptr[i]);
	    }
	  aldebug_printf(NULL,"\n");
	}
      else
	{
	  aldebug_printf(NULL,"NULL datablock ptr in %p\n", block);
	}
    }
}

void aldatablock_setcstring(aldatablock * block,char * cstring)
{
  block->data.charptr = cstring;
  block->length = strlen(cstring);
  block->type = ALTYPE_STR0; 
}

static int alstrings_ringbuffer_get_datablock_internal(alstrings_ringbuffer_pointer ringbuffer, aldatablock * data, int offset)
{
  if ( offset < ringbuffer->bufsize )
    {
      data->type=ALTYPE_OPAQUE;
      data->data.charptr=&ringbuffer->buf[offset];
      data->length=ringbuffer->bufsize-offset;

      return offset;
    }
  return -1;
}

int alstrings_ringbuffer_reserve_datablock(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * data, int bytelength)
{
  // negative offset means reservation did not complete.
  int offset = -1;
  
  if ( ringbufferp != NULL )
    {
      alstrings_ringbuffer_pointer ringbuffer=*ringbufferp;

      if ( ringbuffer != NULL )
	{
	  offset = ringbuffer->bufpos;
	  if ( ringbuffer->bufsize + offset >= bytelength )
	    {
	      alstrings_ringbuffer_get_datablock_internal(ringbuffer, data, offset);
	    }
	  else
	    {
	      ringbuffer = al_token_char_buffer_grow(ringbuffer, bytelength);
	      if ( ringbuffer != NULL )
		{
		  offset = ringbuffer->bufpos;
		  offset = alstrings_ringbuffer_get_datablock_internal(ringbuffer, data,offset);
		  if ( offset >= 0 )
		    {
		      *ringbufferp=ringbuffer;
		    }
		}
	      else
		{
		  offset = -1;
		}
	    }
	}
    }

  return offset;
}

int aldatablock_write_int32be(aldatablock * data, int offset, int word)
{
    // todo check datablock type
  if ( data->length >= offset + 4 )
    {
      char * intern = &data->data.charptr[offset];
      char * hack = (char *) &word;
      // if LittleEndian ask gcc
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      intern[0]=hack[4];
      intern[1]=hack[3];
      intern[2]=hack[2];
      intern[3]=hack[1];
#else
      memcpy(intern,hack,4);
#endif

    }     
	
  return offset + 4;
}
