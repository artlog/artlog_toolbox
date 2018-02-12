#ifndef __ALINPUTSTREAM_HEADER__
#define __ALINPUTSTREAM_HEADER__

#include "alstrings.h"
#include "aldebug.h"

struct alinputstream {
  ALDEBUG_DEFINE_FLAG(debug)
  int fd;
  int eof;
};

ALDEBUG_DECLARE_FUNCTIONS(struct alinputstream,alinputstream)
			 
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
