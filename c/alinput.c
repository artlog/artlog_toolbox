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
  stream->type = ALINPUTSTREAM_TYPE_FD;
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
  return (stream->eof == 1);
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
	  result = alinputstream_readuchar(next);
	}
      else
	{
	  // set eof only if ful chain is eof.
	  alinputstream_seteof(stream);
	  result = 0;
	}
      return result;
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

  // create a new child stream ALLOC on heap
struct alinputstream * alinputstream_share_child_alloc()
{
  struct alinputstream * child_stream = NULL;
  child_stream = (struct alinputstream *) calloc(1,sizeof(*child_stream));
  child_stream->type =  ALINPUTSTREAM_TYPE_SHARED_CHILD;
  return child_stream;
}

void alinputstream_share_child_init(struct alinputstream_share_child * child, struct alinputstream * parent)
{
  child->parent = parent;
  child->offset = 0;
  child->next = NULL;
}

struct alinputstream * alinputstream_find_last_child(struct alinputstream * child)
{
  struct alinputstream * last_child = NULL;
  while ( child != NULL )
    {
      last_child = child;
      child = child->child.self.next;
    }
  return last_child;
}

struct alinputstream * alinputstream_find_previous_child(struct alinputstream * from, struct alinputstream * child)
{
  struct alinputstream * previous_child = NULL;
  while ((from != NULL) && ( from != child ))
    {
      previous_child = from;
      from = from->child.self.next;      
    }
  return previous_child;
}

struct alinputstream * alinputstream_create_mark_shared(struct alinputstream * parent, int blocksize)
{
  struct alinputstream * child_stream = NULL;

  if ( parent->type ==  ALINPUTSTREAM_TYPE_FD )
    {
      child_stream=alinputstream_share_child_alloc();
      struct alinputstream_share_child * child = &child_stream->child.self;
      alinputstream_share_child_init(child,parent);

      // parent is a root stream, ie not a child kind
      if ( parent->input.data.ptr == NULL )
	{
	  parent->type = ALINPUTSTREAM_TYPE_SHARED;
	  // TODO FIXME
	  parent->mark = 0;
	  parent->self_offset = parent->mark;
	  parent->child.ptr = child_stream;

	  // ALLOC will be release at alinputstream_release_shared
	  aldatablock block;
	  char * datablock = (char *) malloc(blocksize);
	  block.length=blocksize;
	  block.data.charptr=datablock;
	  alinputstream_setdatablock(parent, &block, 0);
	}
      else
	{
	  // else means it had buffer ... BAD...
	  aldebug_printf(NULL,"[ERROR] parent buffer unexpected in %s:%s:%i\n", __FILE__,__func__,__LINE__);
	}
    }
  else if ( parent->type ==  ALINPUTSTREAM_TYPE_SHARED )
    {
      struct alinputstream * head_child = parent->child.ptr;
      if (( head_child != NULL ) && ( head_child->child.self.parent == parent ))
	{
	  child_stream=alinputstream_share_child_alloc();
	  struct alinputstream_share_child * child = &child_stream->child.self;
	  alinputstream_share_child_init(child,parent);

	  // parent is a root stream, ie not a child kind
	  aldatablock * block = &parent->input;
	  if ( block->data.ptr != NULL )
	    {
	      // TODO FIXME
	      // chain at end
	      struct alinputstream * last_child = alinputstream_find_last_child(head_child);
	      last_child->child.self.next = child_stream;
	      if ( block->length < blocksize )
		{
		  // TODO reallaoc ? warn ?
		}
	      child->offset=parent->mark;
	    }
	  else
	    {
	      // else means it had buffer ... BAD...
	      aldebug_printf(NULL,"[ERROR] parent buffer shared NULLn %s:%s:%i\n", __FILE__,__func__,__LINE__);
	    }
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] shared stream %p without a child parent set to it %p in %s:%s:%i\n",
			 parent,
			 head_child,
			 __FILE__,__func__,__LINE__);
	}
    }
  else if ( parent->type ==  ALINPUTSTREAM_TYPE_SHARED_CHILD )
    {
      // parent is already a child
      struct alinputstream_share_child * child = &parent->child.self;
      struct alinputstream * shared_parent = child->parent;      
      // RECURSIVE on parent to create a sister or brother
      child_stream = alinputstream_create_mark_shared(shared_parent,blocksize);
      // new child starts where parent is even if parent is a shared child.
      child = &child_stream->child.self;
      child->offset=parent->child.self.offset;
    }
  else
    {     
      aldebug_printf(NULL,"[ERROR] creating a child of child for parent %p type %i child.parent %p in %s:%s:%i\n",
		     parent,
		     parent->type,
		     parent->child.self.parent,
		     __FILE__,__func__,__LINE__);
    }

  if (( child_stream != NULL )&&(child_stream->type != ALINPUTSTREAM_TYPE_SHARED_CHILD))
    {
      aldebug_printf(NULL,"[FATAL] creating a child stream of wrong type %i in %s:%s:%i\n",
		     child_stream->type,
		     __FILE__,__func__,__LINE__);
    }
    
  return child_stream; 
}

void alinputstream_release_shared(struct alinputstream * parent)
{  
  if ( parent->type == ALINPUTSTREAM_TYPE_SHARED )
    {
      aldebug_printf(NULL,"[DEBUG] free shared parent %p in %s:%s:%i\n",
		     parent,
		     __FILE__,__func__,__LINE__);

      if ( parent->child.ptr == NULL )
	{		      
	  if (parent->input.data.ptr != NULL )
	    {
	      free(parent->input.data.ptr);
	      parent->input.data.ptr=NULL;
	      parent->input.length=0;
	      parent->mark=0;
	    }
	  parent->type = ALINPUTSTREAM_TYPE_FD;
	}
    }

}

void alinputstream_free_shared(struct alinputstream * child_stream)
{  
  if ( child_stream != NULL )
    {
      if ( child_stream->type == ALINPUTSTREAM_TYPE_SHARED_CHILD )
	{
	  struct alinputstream_share_child * child = &child_stream->child.self;
	  struct alinputstream * parent = child->parent;
	  if ( parent->type == ALINPUTSTREAM_TYPE_SHARED )
	    {
	      aldebug_printf(NULL,"[DEBUG] free shared child %p in %s:%s:%i\n",
			     child_stream,
			     __FILE__,__func__,__LINE__);

	      if ( parent->child.ptr == child_stream )
		{
		  // this was head, might set head to NULL
		  // release of buffer will be done by parent later with alinputstream_release_shared
		  parent->child.ptr = child->next;
		}
	      else		
		{
		  // this is not head, should remove it from next of its previous.
		  struct alinputstream * previous_child = alinputstream_find_previous_child(parent->child.ptr,child_stream);
		  if ( previous_child != NULL )
		    {
		      previous_child->child.self.next=child->next;
		    }
		}
	    }
	  else
	    {
	      aldebug_printf(NULL,"[ERROR] free shared child %p with invalid parent type %i in %s:%s:%i\n",
			     child_stream,
			     parent->type,
			     __FILE__,__func__,__LINE__);

	    }
	  free(child_stream);
	}
    }
}

unsigned char alinputstream_read_and_record(struct alinputstream * stream, int offset)
{
  int relative = offset - stream->mark;
  if ( relative < 0 )
    {
      aldebug_printf(NULL,"[ERROR] reading before mark in %s:%s:%i\n", __FILE__,__func__,__LINE__);
      return 0;    
    }
  // steam->offset is number of char kept in parent stream after mark.
  if (relative >= stream->offset )
    {
      if ( stream->input.length < relative )
	{
	  aldebug_printf(NULL,"[DEBUG] CAN'T record, buffer too small in %s:%s:%i\n", __FILE__,__func__,__LINE__);
	  return 0;
	}
      unsigned char c = 0;
      // FIXME should read as many characters needed to fill..
      c = alinputstream_readuchar(stream);
      if ( relative != stream->offset )
	{
	  aldebug_printf(NULL,"[ERROR] leaving a hole in parent read %i %i in %s:%s:%i\n",
			 relative,
			 stream->offset,
			 __FILE__,__func__,__LINE__);
	}
      stream->input.data.ucharptr[relative]=c;
      stream->offset=relative+1;
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
  unsigned char result = 0;
  struct alinputstream * stream = NULL;
  if ( childstream->type == ALINPUTSTREAM_TYPE_SHARED_CHILD )
    {
      struct alinputstream_share_child * child = &childstream->child.self;
      stream = child->parent;
      if ( stream != NULL )
	{
	  if ( stream->mark > child->offset )
	    {
	      // we are trying to read at place that has not been record in time !
	      // this is an error
	      // UGLY eof
	      aldebug_printf(NULL,"[ERROR] reading %i  before mark %i in %s:%s:%i\n",
			     child->offset,
			     stream->mark,
			     __FILE__,__func__,__LINE__);	      
	      return 0;
	    }
	  result = alinputstream_read_and_record(stream,child->offset);
	  child->offset++;
	}
    }
  else
    {
      // consider it not shared
      if (  childstream->type == ALINPUTSTREAM_TYPE_SHARED )
	{
	  // TODO FIXME case ALINPUTSTREAM_TYPE_SHARED reading in a shared directly, not though a child ...
	  aldebug_printf(NULL,"[DEBUG] reading directly within ALINPUTSTREAM_TYPE_SHARED %p in %s:%s:%i\n",
			 childstream,
			 __FILE__,__func__,__LINE__);
	  if ( childstream->child.ptr != NULL )
	    {
	      // we still have children
	      result = alinputstream_read_and_record(childstream,childstream->self_offset);
	      childstream->self_offset++;
	    }
	  else if (
		   (childstream->self_offset > childstream->mark )
		   &&
		   (childstream->self_offset < childstream->mark + childstream->offset )
		   )
	    {
	      // did we consume all our buffer ?
	      int relative = childstream->self_offset -childstream->mark;
	      result=childstream->input.data.ucharptr[relative];
	      childstream->self_offset++;
	    }
	  else
	    {
	      
	      result = alinputstream_readuchar(childstream);
	      if (childstream->self_offset >= childstream->mark + childstream->offset )
		{
		  // time to dispose
		  alinputstream_release_shared(childstream);
		}
	    }
	}
      else
	{
	  result = alinputstream_readuchar(childstream);
	}
    }
  
  return result;
}

struct alinputstream *  alinputstream_create_chain(struct alinputstream * current, struct alinputstream * next)
{

  // adding in a NULL will use first as container/start of chain. 
  if ( current == NULL )
    {
      return next;
    }

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
  }
  
  return current;
}

void alinputstream_align_shared_with_child(struct alinputstream * parent, struct alinputstream * childstream)
{

  if ( ( childstream != NULL ) && ( parent != NULL ) )
    {
      if ( childstream->type == ALINPUTSTREAM_TYPE_SHARED_CHILD )
	{
	  struct alinputstream_share_child * child = &childstream->child.self;
	  if ( parent->type ==  ALINPUTSTREAM_TYPE_SHARED_CHILD )
	    {
	      // parent is already a child
	      parent->child.self.offset=child->offset;
	    }
	  else if ( parent->type ==  ALINPUTSTREAM_TYPE_SHARED )
	    {
	      // crossing fingers ...
	      parent->self_offset=child->offset;
	    }
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] can't align parent %p on a child %p that is not a ALINPUTSTREAM_TYPE_SHARED_CHILD but %i  in %s:%s:%i\n",
			 parent,
			 childstream,
			 childstream->type,
			 __FILE__,__func__,__LINE__);	      

	}
    }
}
