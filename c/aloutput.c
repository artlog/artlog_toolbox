#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "aloutput.h"
#include "aldebug.h"

void aloutputstream_init(struct aloutputstream * stream, FILE * file)
{
  if ( file != NULL )
    {
      stream->file=file;
      stream->fd = fileno(file);
      stream->target=ALOUTPUT_TARGET_FILE;
    }
  else
    {
      stream->file=NULL;
      stream->fd=-1;
    }
  stream->callback_writeint32 = NULL;
  stream->callback_flush = NULL;
  stream->callback_close = NULL;
}

void aloutputstream_init_shared_buffer(struct aloutputstream * stream, char * buffer)
{
  stream->file= NULL;
  stream->fd = -1;
  stream->data=buffer;
  stream->callback_writeint32 = NULL;
  stream->callback_flush = NULL;
  stream->callback_close = NULL;
  stream->target=ALOUTPUT_TARGET_BUFFER;
}

void aloutputstream_set_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_writeint32 callback_writeint32,
				 aloutput_callback_flush callback_flush,
				 aloutput_callback_close callback_close)
{
  stream->callback_writeint32 = callback_writeint32;
  stream->callback_flush = callback_flush;
  stream->callback_close = callback_close;
}

int alaloutputstream_getfd(struct aloutputstream * stream)
{
  return stream->fd;
}

FILE * aloutputstream_file(struct aloutputstream * stream)
{
  return stream->file;
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
      aldebug_printf(NULL,"[ERROR] %s %u wrote %i\n","error when writing uint32 ", word, r);
    }
}

static int aloutputstream_isfile( struct aloutputstream * stream )
{
  return ( stream->target == ALOUTPUT_TARGET_FILE );
}

void aloutputstream_writeint32(struct aloutputstream * stream, int word)
{
  if ( stream->callback_writeint32 != NULL )
    {
      (*stream->callback_writeint32)(stream,word);
    }
  else
    {
      if ( aloutputstream_isfile(stream) )
	{
	  aloutputstream_writeint32_fd(stream,word,stream->fd,4);
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
      if ( aloutputstream_isfile(stream) )
	{
	  if ( bits > 0 )
	    {
	      int bytes = ((bits-1) / CHAR_BIT) + 1;
	      aldebug_printf(NULL,"last pad to byte %i\n", bytes);
	      aloutputstream_writeint32_fd(stream,word,stream->fd, bytes);
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
      if ( aloutputstream_isfile(stream) )
	{
	  if ( stream->file != NULL )
	    {
	      fclose(stream->file);
	      stream->fd=-1;
	    }
	  else
	    {
	      if (stream->fd >=0)
		{
		  close(stream->fd);
		}
	    }	
	}
    }
}
