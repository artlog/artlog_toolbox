#include "aloutput.h"
#include "aldebug_output.h"
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>

void aloutputstream_fd_init(struct aloutputstream * stream, int fd)
{
  stream->fd = fd;
  stream->target=ALOUTPUT_TARGET_FD;
  stream->callback_write_byte = NULL;
  stream->callback_writeint32 = NULL;
  stream->callback_flush = NULL;
  stream->callback_close = NULL;
}

void aloutputstream_init_shared_buffer(struct aloutputstream * stream, aldatablock * buffer, int offset)
{
  stream->target=ALOUTPUT_TARGET_BUFFER;
  stream->fd = -1;
  memcpy(&stream->buffer,buffer,sizeof(stream->buffer));
  stream->offset=0;
  stream->callback_write_byte = NULL;
  stream->callback_writeint32 = NULL;
  stream->callback_flush = NULL;
  stream->callback_close = NULL;
}

void aloutputstream_set_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_write_byte callback_write_byte,				 
				 aloutput_callback_writeint32 callback_writeint32,
				 aloutput_callback_flush callback_flush,
				 aloutput_callback_close callback_close)
{
  stream->callback_write_byte = callback_write_byte;
  stream->callback_writeint32 = callback_writeint32;
  stream->callback_flush = callback_flush;
  stream->callback_close = callback_close;
}

void aloutputstream_set_close_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_close callback_close,
				 void * data)
{
  stream->callback_close = callback_close;
  stream->data=data;
}

void aloutputstream_writeint32_fd(struct aloutputstream * stream, int word, int fd, int bytes)
{
  int r = 0;
  char * wp = (char *)  &word;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  // warning internal representation wp little endian.
  char w[4];
  w[0]=wp[3];
  w[1]=wp[2];
  w[2]=wp[1];
  w[3]=wp[0];
  r=write(fd,w,bytes);
#else
  r=write(fd,wp,bytes);
#endif
  if ( r != bytes)
    {
      aldebug_printf(DBGSTREAM,"[ERROR] %s %u wrote %i\n","error when writing uint32 ", word, r);
    }
}


static int aloutputstream_is_fd( struct aloutputstream * stream )
{
  return ( stream->target == ALOUTPUT_TARGET_FD );
}

// hack, file is deprecated, only fd is supported
static int aloutputstream_is_file( struct aloutputstream * stream )
{
  return aloutputstream_is_fd(stream);
}

static int aloutputstream_is_buffer( struct aloutputstream * stream )
{
  return ( stream->target == ALOUTPUT_TARGET_BUFFER );
}

void aloutputstream_write_byte(struct aloutputstream * stream, unsigned char byte)
{
  if ( stream->callback_write_byte != NULL )
    {
      (*stream->callback_write_byte)(stream,byte);
    }
  else
    {
      if ( aloutputstream_is_file(stream) )
	{
	  // write a byte ...
	  write(stream->fd,&byte,1);
	}
      else if ( aloutputstream_is_buffer(stream) )
	{
	  stream->offset = aldatablock_write_byte(&stream->buffer, stream->offset, byte);
	}
    }
}

void aloutputstream_writeint32(struct aloutputstream * stream, int word)
{
  if ( stream->callback_writeint32 != NULL )
    {
      (*stream->callback_writeint32)(stream,word);
    }
  else
    {
      if ( aloutputstream_is_file(stream) )
	{
	  aloutputstream_writeint32_fd(stream,word,stream->fd,4);
	}
      else if ( aloutputstream_is_buffer(stream) )
	{
	  stream->offset = aldatablock_write_int32be(&stream->buffer, stream->offset, word);
	}
    }
}


void aloutputstream_flush(struct aloutputstream * stream, int word, int bits)
{
  if ( stream->callback_flush != NULL )
    {
      (*stream->callback_flush)(stream,word,bits);
    }  
  else
    {
      if ( bits > 0 )
	{
	  int bytes = ((bits-1) / CHAR_BIT) + 1;
	  aldebug_printf(DBGSTREAM,"last pad to byte %i\n", bytes);      
	  if ( aloutputstream_is_file(stream) )
	    {
	      aloutputstream_writeint32_fd(stream,word,stream->fd, bytes);
	    }
	  else if ( aloutputstream_is_buffer(stream) )
	    {
	      // write full word but truncate last offset.
	      int offset = stream->offset;
	      aldatablock_write_int32be(&stream->buffer, stream->offset, word);
	      stream->offset = offset + bytes;
	    }
	}
    }
}

void * aloutputstream_get_data(struct aloutputstream * stream)
{
  return stream->data;
}

void aloutputstream_close(struct aloutputstream * stream)
{
  if ( stream->callback_close != NULL )
    {
      (*stream->callback_close)(stream);
    }
  else
    {
      if ( aloutputstream_is_file(stream) )
	{
	  /**
	  if ( stream->file != NULL )
	    {
	      fclose(stream->file);
	      stream->fd=-1;
	    }
	  else
	  */
	    {
	      if (stream->fd >=0)
		{
		  close(stream->fd);
		}
	    }	
	}
    }
}


int aloutputstream_memcpy(int written, struct aloutputstream * stream, char * localbuffer)
{
   if (written > 0)
    {
      if ( aloutputstream_is_file( stream ) )
	{
	  write(stream->fd,localbuffer,written);	  
	}
      else if ( aloutputstream_is_buffer(stream) )
	{
	  int offset = stream->offset;
	  if ( offset + written < stream->buffer.length)
	    {
	      memcpy(stream->buffer.data.charptr+offset,localbuffer,written);
	      stream->offset+=written;
	    }
	}
    }
  
  return written;
}
  
int aloutputstream_printf_1k(struct aloutputstream * stream, const char *format, ...)
{
  char localbuffer[1024];
  int written=0;

  va_list args;
  va_start(args, format);
  written=vsnprintf(localbuffer, 1024, format, args);
  va_end(args);

  return aloutputstream_memcpy(written, stream, localbuffer);

}

int aloutputstream_vprintf_1k(struct aloutputstream * stream, const char *format, va_list args)
{
  char localbuffer[1024];
  int written=0;

  written=vsnprintf(localbuffer, 1024, format, args);

  return aloutputstream_memcpy(written, stream, localbuffer);
}

const char * hexchars = "0123456789abcdef";

// order 1 or 0
// 1 : big endian, 0 little endian
// group = 4 word, 8 long word ...
void aloutput_bytes_as_hex(struct aloutputstream * stream,  aldatablock * datablock, int order, int group)
{
  unsigned char *chr_a = datablock->data.ucharptr;
  int len = datablock->length;
  
  // 1=>1 0=>-1;
  int step = order * 2 -1;

  /*
  int last = len % group;
  if ( last != 0 )
    {
      aldebug_printf(DBGSTREAM,"using a length '%i' that is not a mutliple of group '%i'",len, group);
    }
  */
  
  // 1 : 0 .. len-1
  // -1 : (len-1) .. 0   
  for (int i = 0; i < len; i+=group)
    {
      // 1 => i , 0  => i + group - 1;
      int start =  i + ( group - 1 )  * ( 1 - order );      
      for (int j = 0; j < group; j++)
	{
	  int index = start + step * j;	  
	  unsigned char uc = '\0';
	  if ( index < len )
	    {
	      uc = chr_a[ index ];
	    }
	  int higher = (int) (uc >> 4);
	  int lower = (int) (uc & 0xf);       
	  aloutputstream_write_byte(stream, hexchars[higher]);
	  aloutputstream_write_byte(stream, hexchars[lower]);
	}
    }
}

