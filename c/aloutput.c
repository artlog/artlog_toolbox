#include <unistd.h>
#include <stdio.h>
#include "aloutput.h"
#include "aldebug.h"

void aloutputstream_init(struct aloutputstream * stream, FILE * file)
{
  stream->file=file;
  stream->fd = fileno(file);
}

int alaloutputstream_getfd(struct aloutputstream * stream)
{
  return stream->fd;
}

FILE * aloutputstream_file(struct aloutputstream * stream)
{
  return stream->file;
}

void aloutputstream_writeint32(struct aloutputstream * stream, int word)
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
  r=write(stream->fd,w,4);
#else
  r=write(stream->fd,wp,4);
#endif
  if ( r != 4)
    {
      aldebug_printf(NULL,"[ERROR] %s %u wrote %i\n","error when writing uint32 ", word, r);
    }   
}
