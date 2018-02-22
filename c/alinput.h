#ifndef __ALINPUTSTREAM_HEADER__
#define __ALINPUTSTREAM_HEADER__

#include "alstrings.h"
#include "aldebug.h"

struct alinputstream {
  ALDEBUG_DEFINE_FLAG(debug)
  int fd;
  int eof;
  int bits; // last bits read during last operation ( with eof )
  // memory inputstream.
  struct alhash_datablock input;
  // offset within input
  int offset;
};

ALDEBUG_DECLARE_FUNCTIONS(struct alinputstream,alinputstream)
			 
void alinputstream_init(struct alinputstream * stream, int fd);

/**
if 4 bytes can't be read, result eof will be set
and bits will be set to number of bits read ( a multiple of 8 obvioulsy )
in which case returned value is relevant for highter most bits
*/
unsigned int alinputstream_readuint32(struct alinputstream * stream);

int alinputstream_get_readbits(struct alinputstream * stream);

unsigned char alinputstream_readuchar(struct alinputstream * stream);

void alinputstream_foreach_block(
				 struct alinputstream * stream,
				 int blocksize,
				 void (*callback) (struct alhash_datablock * block, void * data),
				 void (*finalize) (struct alhash_datablock * block, void * data),
				 void * data);

/** will read in memory from datablock starting at offset byte */
void alinputstream_setdatablock(struct alinputstream * stream, struct alhash_datablock * block, int offset);

#endif
