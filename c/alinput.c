#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "alinput.h"
#include "aldebug.h"

ALDEBUG_DEFINE_STRUCT_FUNCTIONS(alinputstream)

void alinputstream_init(struct alinputstream * stream, int fd)
{
  bzero(stream, sizeof(*stream));
  stream->fd=fd;
  stream->input.data.ptr=NULL;  
}

void alinputstream_setdatablock(struct alinputstream * stream, struct alhash_datablock * block, int offset)
{
  memcpy(&stream->input, block, sizeof(stream->input));
  stream->offset=offset;
}

unsigned int alinputstream_readuint32(struct alinputstream * stream)
{
  if ( stream->input.data.ptr != NULL )
    {
      unsigned int res = 0;
      if (  stream->input.length >= stream->offset + 4 )
	{
	  res = aldatablock_get_uint32be(&stream->input, stream->offset);
	  stream->offset += 4;
	}
      else
	{
	  int remain = stream->input.length - stream->offset;
	  if ( remain > 0 )
	    {

	      int d =0;
	      for (d=0; d<remain;d++)
		{
		  res = ( res * 256 ) + stream->input.data.ucharptr[d];
		}
	      /** align to msb ??
	      for (d<4;d++)
		{
		  res *= 256;		  
		}
	      */
	      stream->bits = remain * CHAR_BIT;
	    }
	  stream->eof=1;
	}
      
      return res;
    }
  else
    {
      unsigned char v[4];
      unsigned char result[4];
      size_t total = 0;
      size_t r = 0;

      // handle case where 4 bytes are read in multiple chunks (often network issues)
      while (total < 4)
	{
	  r = read(stream->fd, &v + total, 4 - total);
	  // 4 bytes ints
	  if ( r > 0 )
	    {
	      total = total + r;
	    }
	  else
	    {
	      stream->bits = total * CHAR_BIT;
	      stream->eof=1;
	      break;
	    }
	}
      // reverse big endian => little endian ( internal intel int )
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      result[0]=v[3];
      result[1]=v[2];
      result[2]=v[1];
      result[3]=v[0];
#else
      memcpy(result,v,4);
#endif
  
      ALDEBUG_IF_DEBUG(stream,alinputstream,debug)
	{
	  aldebug_printf(NULL,"%lu %x %x %x %x\n",total, v[0], v[1], v[2], v[3]);
	}  
      return (*(unsigned int*) result);
    }
}

// WARNING 0 char considered as EOF.
unsigned char alinputstream_readuchar(struct alinputstream * stream)
{
  unsigned char result = 0;
  if ( read(stream->fd,&result,1) == 1 )
    {
      return result;
    }
  else
    {
      return 0;
    }
}

int alinputstream_read_block(struct alinputstream * stream,
			     struct alhash_datablock * block)
{  
  return read(stream->fd,block->data.charptr,block->length);
}

void alinputstream_foreach_block(
				 struct alinputstream * stream,
				 int blocksize,
				 void (*callback) (struct alhash_datablock * block, void * data),
				 void (*finalize) (struct alhash_datablock * block, void * data),
				 void * data)
{
  struct alhash_datablock block;
  char * datablock = (char *) malloc(blocksize);
  block.length=blocksize;
  
  if ( datablock != NULL )
    {
      block.data.ptr=datablock;
      int read = 0;
      while ( ( read = alinputstream_read_block(stream, &block) ) == blocksize )
	{
	  (*callback) (&block,data);
	}
      block.length=read;
      (*finalize) (&block,data);
    }
  
}

int alinputstream_get_readbits(struct alinputstream * stream)
{
  return stream->bits;
}
