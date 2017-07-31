#ifndef _OUTPUTSTREAM_H_
#define _OUTPUTSTREAM_H_

#include <stdio.h>

struct outputstream {
  FILE * file;
  int fd;
  int debug;
};

void outputstream_init(struct outputstream * stream, FILE * file);

void outputstream_writeint32(struct outputstream * stream, int word);

int outputstream_getfd(struct outputstream * stream);

FILE * outputstream_file(struct outputstream * stream);
#endif
