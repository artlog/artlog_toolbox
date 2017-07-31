#include <unistd.h>
#include <stdio.h>
#include "outputstream.h"

void outputstream_init(struct outputstream * stream, FILE * file)
{
  stream->file=file;
  stream->fd = fileno(file);
}

int outputstream_getfd(struct outputstream * stream)
{
  return stream->fd;
}

FILE * outputstream_file(struct outputstream * stream)
{
  return stream->file;
}

void outputstream_writeint32(struct outputstream * stream, int word)
{
  // warning little endian !
  int r = 0;
  char w[4];
  char * wp = (char *)  &word;
  w[0]=wp[3];
  w[1]=wp[2];
  w[2]=wp[1];
  w[3]=wp[0];
  r=write(stream->fd,w,4);
  if ( r != 4)
    {
      printf("%s %u\n","error when writing uint32 ", word);
    }
   
}
