#ifndef _OUTPUTSTREAM_H_
#define _OUTPUTSTREAM_H_

#include <stdio.h>

struct aloutputstream {
  FILE * file;
  int fd;
  int debug;
};

void aloutputstream_init(struct aloutputstream * stream, FILE * file);

void aloutputstream_writeint32(struct aloutputstream * stream, int word);

int aloutputstream_getfd(struct aloutputstream * stream);

FILE * aloutputstream_file(struct aloutputstream * stream);
#endif
