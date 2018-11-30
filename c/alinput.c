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

void alinputstream_setdatablock(struct alinputstream * stream, aldatablock * block, int offset)
{
  memcpy(&stream->input, block, sizeof(stream->input));
  stream->offset=offset;
}

void alinputstream_seteof(struct alinputstream * stream)
{
  stream->eof=1;
}

int alinputstream_iseof(struct alinputstream * stream)
{
  return stream->eof;
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
	  alinputstream_seteof(stream);
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
	      alinputstream_seteof(stream);
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

// WARNING 0 char considered as EOF
unsigned char alinputstream_readuchar(struct alinputstream * stream)
{
  unsigned char result = 0;
  if ( alinputstream_iseof(stream) )
    {
      return 0;
    }

  if ( read(stream->fd,&result,1) == 1 )
    {
      return result;
    }
  else
    {
      struct alinputstream * next = stream->next_chain;
      if ( ( next != NULL ) && ( ! alinputstream_iseof(next)))
	{
	  result = alinputstream_readuchar(stream);
	}
      else
	{
	  // set eof only if ful chain is eof.
	  alinputstream_seteof(stream);
	}
      return 0;
    }
}

int alinputstream_read_block(struct alinputstream * stream,
			     aldatablock * block)
{  
  return read(stream->fd,block->data.charptr,block->length);
}



void alinputstream_foreach_block(
				 struct alinputstream * stream,
				 int blocksize,
				 void (*callback) (aldatablock * block, void * data),
				 void (*finalize) (aldatablock * block, void * data),
				 void * data)
{
  aldatablock block;
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

struct alinputstream * alinputstream_create_mark_shared(struct alinputstream * parent, int blocksize)
{
  // FIXME currently allow only one child ...
  if ( parent->child.parent == NULL )
    {
      struct alinputstream_share_child * child = &parent->child;

      if ( parent->input.data.ptr == NULL )
	{
	  // TODO FIXME
	  parent->mark = 0;
      
	  aldatablock block;
	  char * datablock = (char *) malloc(blocksize);
	  block.length=blocksize;
	  block.data.charptr=datablock;
	  alinputstream_setdatablock(parent, &block, 0);
	}      
      // else means it had buffer ... BAD...

      child->parent=parent;
    }

  return parent; 
}

void alinputstream_free_shared(struct alinputstream * child)
{
  // FIXME currently allow only one child ...  
  struct alinputstream * parent = child;
  if (parent->input.data.ptr != NULL )
    {
      free(parent->input.data.ptr);
      parent->input.data.ptr=NULL;
      parent->input.length=0;
    }
}

unsigned char alinputstream_read_and_record(struct alinputstream * stream, int offset)
{
  int relative = stream->mark - offset;
  // steam->offset is number of char kept in parent stream
  if (relative > stream->offset )
    {
      if ( stream->input.length < relative )
	{
	  aldebug_printf(NULL,"[DEBUG] CAN'T record, buffer too small in %s:%s:%i\n", __FILE__,__func__,__LINE__);
	  return 0;
	}
      unsigned char c = 0;
      c = alinputstream_readuchar(stream);
      stream->input.data.ucharptr[relative]=c;      
      return c;
    }
  else
    {
      return stream->input.data.ucharptr[relative];
    }
}

// WARNING 0 char considered as EOF.
unsigned char alinputstream_shared_readuchar(struct alinputstream * childstream)
{
  
  struct alinputstream_share_child * child = &childstream->child;
  struct alinputstream * stream = childstream->child.parent;
  unsigned char result = 0;

  if ( stream != NULL )
    {
      if ( stream->mark >= child->offset )
	{
	  // we are trying to read at place that has not been record in time !
	  // this is an error
	  // UGLY eof
	  return 0;
	}

      result = alinputstream_read_and_record(stream,child->offset);
    }
  else
    {
      // in facts not shared
      result = alinputstream_readuchar(childstream);
    }
  
  return result;
}

struct alinputstream *  alinputstream_create_chain(struct alinputstream * current, struct alinputstream * next)
{
  struct alinputstream * last = current;

  while ( last != NULL )
    {
      if ( last->next_chain != NULL )
	{
	  last = last->next_chain;
	}
      else
	{
	  last->next_chain = next;
	  last = NULL;
	}
    }
  
  return current;
}
