#ifndef __ALINPUTSTREAM_HEADER__
#define __ALINPUTSTREAM_HEADER__

#include "alstrings.h"

struct alinputstream {
  int fd;
  int eof;
  int debug;
};

void alinputstream_init(struct alinputstream * stream, int fd);

unsigned int alinputstream_readuint32(struct alinputstream * stream);

unsigned char alinputstream_readuchar(struct alinputstream * stream);

void alinputstream_foreach_block(
				 struct alinputstream * stream,
				 int blocksize,
				 void (*callback) (struct alhash_datablock * block, void * data),
				 void (*finalize) (struct alhash_datablock * block, void * data),
				 void * data);

#endif
