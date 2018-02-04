#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "alinput.h"
#include "aldebug.h"

void alinputstream_init(struct alinputstream * stream, int fd)
{
  stream->fd=fd;
  stream->eof=0;
  stream->debug=0;
}

unsigned int alinputstream_readuint32(struct alinputstream * stream)
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
      aldebug_printf(NULL,"%lu %x %x %x %x\n",total, v[0], v[1], v[2], v[3]);
    }  
  return (*(unsigned int*) result);
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
