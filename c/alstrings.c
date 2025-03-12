#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alstrings.h"
#include "alcommon.h"
#include "aldebug.h"
#include "aldebug_output.h"


enum alstrings_debug_flags {
  ALSTRINGS_DEBUG_FLAG = 1,
};

static int alstrings_debug_flags = 0;

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

/* construct circular linked list at once within an array of size times.
   allocated on heap and zeroed
   return first
*/
alstrings_ringbuffer_pointer al_alstrings_ringbuffer_alloc(int times)
{
  alstrings_ringbuffer_pointer buffers = calloc(times, sizeof(alstrings_ringbuffer));
  for (int i=0; i<times;i++)
    {
      buffers[i].first = buffers;
      // last will point on first circular.
      buffers[i].next = &buffers[(i+1)%times];
    }
  return buffers;
}

// allocate buffer of chars bytes on heap not zeroed.
void al_alstrings_ringbuffer_alloc_internal(alstrings_ringbuffer_pointer ringbuffer, int chars)
{
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  buffer->bufsize = chars;
  buffer->bufpos = 0;
  buffer->buf = calloc(1,buffer->bufsize);
  ringbuffer->canary = ALSTRINGBUFCANARY;
  // can be freed with int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
  // don't set first or next
}

// get last pointing on circular.
alstrings_ringbuffer_pointer  al_alstrings_ringbuffer_get_previous(alstrings_ringbuffer_pointer circular)
{
  alstrings_ringbuffer_pointer buffer = circular;
  alstrings_ringbuffer_pointer previous = circular;
  alstrings_ringbuffer_pointer next = buffer->next;
  // HARDCODED max buckets 1000
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
	  aldebug_printf(DBGSTREAM,"[FATAL] long loop (infinite ? ) on  al_alstrings_ringbuffer_get_previous\n");
	}
      // HARD EXIT
      exit(1);
    }
  return previous;
}

/* pick from buffer one that can provide length
   if not enough place create a new one on heap filled with zeros
*/
alstrings_ringbuffer_pointer al_alstrings_ringbuffer_grow(alstrings_ringbuffer_pointer ringbuffer, int length)
{
  alstrings_ringbuffer_pointer next  = ringbuffer->next;
  struct alstrings_buffer * buffer = &ringbuffer->buffer;
  int bufsize = buffer->bufsize * 2;

  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      aldebug_printf(DBGSTREAM,"grow alstrings_ringbuffer %p %i/%i\n", buffer, buffer->bufpos, buffer->bufsize);
    }
  // last point on first; this is circular
  while ( ( next != NULL ) && ( next != ringbuffer ) )
    {
      if ( next->buffer.buf == NULL )
	{
	  // grown
	  // at least length...
	  if ( bufsize < length )
	    {
	      bufsize = length;
	    }
	  al_alstrings_ringbuffer_alloc_internal(next, bufsize);
	  // what about first and next_free ?
	  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
	    {
	      aldebug_printf(DBGSTREAM,"allocate new next alstrings_ringbuffer %p %i/%i\n", next, next->buffer.bufpos, next->buffer.bufsize);
	    }
	  return next;
	}
      else
	{
	  // found a place, no need to grow
	  if ( ( next->buffer.bufsize - next->buffer.bufpos ) >= length )
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{
		  aldebug_printf(DBGSTREAM,"found place next alstrings_ringbuffer %p %i/%i\n", next, next->buffer.bufpos, next->buffer.bufsize);
		}
	      return next;
	    }
	}
      if (next->buffer.bufsize > 0 )
	{
	  bufsize = next->buffer.bufsize * 2;
	}
      next = next->next;
    }

  // what to conclude here if next == buffer ?
  // ==> that it was not possible to allocate a buffer
  if (  next == ringbuffer )
    {
      aldebug_printf(DBGSTREAM,"[FATAL] not possible to allocate a buffer. allocate buffer (%p) next (%p) length (%i) \n", ringbuffer, next,bufsize);
      return NULL;
    }

  if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
    {
      aldebug_printf(DBGSTREAM,"allocate buffer (%p) next (%p) !\n", buffer, next);
    }

  if ( ((unsigned long long) next) < 1024 )
    {
      fprintf(stderr,"[FATAL] very small buffer pointer %p buffer => bug ?\n", buffer);
    }

  return next;
}

// newly allocated (on heap) block is filled with zeros
char * al_alloc_block(alstrings_ringbuffer_pointer * ringbufferp, int length)
{
  if ( ringbufferp != NULL )
    {
      alstrings_ringbuffer_pointer ringbuffer = (*ringbufferp);
      if ( ringbuffer->canary !=  ALSTRINGBUFCANARY )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] wrong allocation buffer %p, wrong canary %x\n",ringbuffer, ringbuffer->canary);	  
	}
      if ( length > 0 )
	{
	  struct alstrings_buffer * buffer = &ringbuffer->buffer;
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
		  ringbuffer = al_alstrings_ringbuffer_grow(ringbuffer, length);	  
		  if ( ringbuffer != NULL )
		    {
		      buffer = &ringbuffer->buffer;
		      if ( ((unsigned long long) ringbuffer) < 1024 )
			{
			  fprintf(stderr,"[FATAL] very small buffer pointer %p buffer => bug ?\n", buffer);
			}
		      // could consider this bucket as new head to not walk from start always
		      (*ringbufferp) = ringbuffer;
		    }
		  else {
		    // allocation failed indeed
		    return NULL;
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
		      aldebug_printf(DBGSTREAM,"[FATAL] token char buffer allocation shortage");
		    }
		  exit(1);
		}
	    }
	  else
	    {
	      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
		{
		  aldebug_printf(DBGSTREAM,"[FATAL] token char buffer is NULL");
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
      alstrings_ringbuffer_pointer buffer = (*ringbufferp);
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
		  aldebug_printf(DBGSTREAM,"[FATAL] token char buffer allocation shortage");
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
      alstrings_ringbuffer_pointer allocated = al_alstrings_ringbuffer_alloc(buckets);
      al_alstrings_ringbuffer_alloc_internal(allocated,firstbucketlength);
      if ( alstrings_debug_flag_is_set(ALSTRINGS_DEBUG_FLAG) )
	{
	  aldebug_printf(DBGSTREAM,"[DEBUG] allocated %p\n", allocated);
	}
      *ringbufferp = allocated;
     }
}

int alstrings_freebucket(alstrings_ringbuffer_pointer bucket, int count, void * data)
{
  if ( bucket != NULL )
    {
      // see what is done in void al_alstrings_ringbuffer_alloc_internal(alstrings_ringbuffer_pointer buffer, int chars)
      if (bucket->buffer.buf != NULL )
	{
	  aldebug_printf(DBGSTREAM,"[DEBUG] free bucket %p\n", bucket->buffer.buf);
	  free( bucket->buffer.buf);
	}
      bucket->buffer.bufsize = 0;
      bucket->buffer.bufpos = 0;
      // Shouldn"t we flag it as freed in canary : nope since kind might not have changed?
      // set buf as NULL ?
      bucket->buffer.buf = NULL;
    }
  // continue
  return 0;
}

void alstrings_ringbuffer_walk_buckets(alstrings_ringbuffer_pointer ringbuffer, int (*callback) (alstrings_ringbuffer_pointer bucket, int count, void * data), void * data)
{
  alstrings_ringbuffer_pointer buffer = ringbuffer;
  alstrings_ringbuffer_pointer previous = NULL;
  alstrings_ringbuffer_pointer next = buffer->next;
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
      alstrings_ringbuffer_pointer torelease = (*ringbufferp);
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

void aldatablock_fill_uchar(aldatablock * data,int offset, int length, unsigned char fill)
{
  if ( fill == 0 )
    {
      aldatablock_bzero(data,offset,length);
    }
  else
    {
      // todo check datablock type
      if ( ( offset >=0 ) && ( data->length >= offset + length ) )
	{
	  unsigned char * uchar_data = data->data.ucharptr;
	  if (uchar_data != NULL)
	    {
	      for ( int i = offset; i < length + offset; i++ )
		{
		  uchar_data[i] = fill;
		}
	    }
	}
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
      // why 8 ?
      #error untested
      memcpy(intern,hack,8);
#endif
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[ERROR] get int out of bound %i/%i\n", offset,data->length);
    }
  return result;
}

// read an unsigned int that was stored in little endian at offset in datablock
unsigned int aldatablock_get_uint32le(aldatablock * data, int offset)
{
  // todo check datablock type
  unsigned int result = 0;
  if ( data->length >= offset + 4)
    {
      unsigned char * intern = (unsigned char *) &result;
      unsigned char * hack = &data->data.ucharptr[offset];

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      // why 8 ?
      memcpy(intern,hack,8);
#else
      intern[0]=hack[3];
      intern[1]=hack[2];
      intern[2]=hack[1];
      intern[3]=hack[0];
#endif
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[ERROR] get int out of bound %i/%i\n", offset,data->length);
    }
  return result;
}

void aldatablock_setcstring(aldatablock * block,char * cstring)
{
  block->data.charptr = cstring;
  // TODO should be + 1 to include trailing '\0'
  block->length = strlen(cstring);
  block->type = ALTYPE_STR0;
}

static int alstrings_ringbuffer_get_datablock_internal(alstrings_ringbuffer_pointer ringbuffer, aldatablock * data, int offset)
{
  if ( offset < ringbuffer->buffer.bufsize )
    {
      data->type=ALTYPE_OPAQUE;
      data->data.charptr=&ringbuffer->buffer.buf[offset];
      data->length=ringbuffer->buffer.bufsize-offset;

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
	  offset = ringbuffer->buffer.bufpos;
	  if ( ringbuffer->buffer.bufsize + offset >= bytelength )
	    {
	      alstrings_ringbuffer_get_datablock_internal(ringbuffer, data, offset);
	    }
	  else
	    {
	      ringbuffer = al_alstrings_ringbuffer_grow(ringbuffer, bytelength);
	      if ( ringbuffer != NULL )
		{
		  if ( ((unsigned long long) ringbuffer) < 1024 )
		    {
		      fprintf(stderr,"[FATAL] very small buffer pointer %p buffer => bug ?\n", ringbuffer);
		    }
		  offset = ringbuffer->buffer.bufpos;
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

char * alstrings_copy_str_block(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * str)
{
  if ( str->type == ALTYPE_SUBSTR )
    {
      // copy it and add a final NUL
      str->type = ALTYPE_STR0;
      str->length += 1;
    }
  // todo reserve for last NUL should be an extra param of copy block ( alloc more than copy ).
  // using al_copy_block allows to have data block autogrowth.
  char * newstr=al_copy_block(ringbufferp, str);
  if ( ( newstr != NULL ) && ( str->type == ALTYPE_STR0 ) )
    {
      newstr[str->length-1] = '\0';
    }
  str->data.charptr=newstr;
  return newstr;
}

int alstrings_compare_str0_substr(aldatablock * str0, aldatablock * substr)
{
  if ( str0->length == (substr->length + 1) )
    {
      if ( str0->data.ptr == substr->data.ptr )
	{
	  return 0;
	}
      if ( (str0->data.ptr != NULL) && ( substr->data.ptr != NULL ) )
	{
	  return memcmp(str0->data.ptr, substr->data.ptr, substr->length) ;
	}
    }
  return 1;
}


static int hexchar_to_int(char a)
{
  int x = (a > '9') ? 10 + a - 'a'  : a - '0';
  return x;
}

// to move to input
unsigned char alstrings_hex_to_byte(char a, char b)
{

  unsigned char byte = (unsigned char) ( 16 * hexchar_to_int(a) + hexchar_to_int(b) ) ;
  return byte;
}


int alstrings_buffer_add_char(struct alstrings_buffer * buffer, char c, int newbufsize)
{
  int bufsize=newbufsize;
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
      /*
      if ( bufsize > ALTOKEN_BUFSIZE_MAX )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_MAX);
	  exit(0);
	}
      if ( bufsize > ALTOKEN_BUFSIZE_WARNING )
	{
	  aldebug_printf(DBGSTREAM,"[WARNING] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_WARNING);
	}
      */
      if (alstring_grow_buffer_if_needed(buffer,bufsize) == -1)
	{
	  aldebug_printf(DBGSTREAM,"FATAL memory shortage in %s %s %i size %i\n", __FILE__, __FUNCTION__, __LINE__, bufsize );
	  return -1;
	}
      else
	{
	  //safeguard provision for C string printf %s
	  buffer->buf[bufsize-1]=0;
	}
    }
  buffer->buf[buffer->bufpos++]=c;
  return 0;
}

void alstrings_buffer_flush(struct alstrings_buffer * buffer)
{
  if (buffer->buf != NULL )
    {
      free(buffer->buf);
      bzero(buffer, sizeof(*buffer));
    }
}

int alstring_grow_buffer_if_needed(struct alstrings_buffer * buffer, int newsize)
{
  // hold result of this function too
  int oldsize = buffer->bufsize;
  if ( newsize > oldsize )
    {
      char * newbuf=realloc(buffer->buf,newsize);
      if ( newbuf != NULL )
	{
	  buffer->buf=newbuf;
	  buffer->bufsize=newsize;
	}
      else
	{
	  oldsize = -1;
	}
    }
  return oldsize;
}

int alstring_prefix(char * txt, char* keyword, int keywordlength, int match, int miss)
{
  int value = miss;
  for (int i = 0; i < keywordlength; i++)
    {
      if ( txt[i] == 0 )
	{
	  return value;
	}
      if ( txt[i] == keyword[i] )
	{
	  value = match;
	}
      else
	{
	 return  miss;
	}
    }
  return ( txt[keywordlength] == 0 ) ? value : miss;
}
