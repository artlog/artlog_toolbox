#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "inputstream.h"

void inputstream_init(struct inputstream * stream, int fd)
{
  stream->fd=fd;
  stream->eof=0;
  stream->debug=0;
}

unsigned int inputstream_readuint32(struct inputstream * stream)
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
	  stream->eof=1;
	  return 0;
	}
    }
  // reverse big endian => little endian ( internal intel int )
  result[0]=v[3];
  result[1]=v[2];
  result[2]=v[1];
  result[3]=v[0];
  if ( stream->debug )
    {
      fprintf(stderr,"%lu %x %x %x %x\n",total, v[0], v[1], v[2], v[3]);
    }  
  return (*(unsigned int*) result);
}

unsigned char inputstream_readuchar(struct inputstream * stream)
{
  unsigned char result = 0;
  read(stream->fd,&result,1);
  return result;
}
