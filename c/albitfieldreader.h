#ifndef __ALBITFIELDREADER_H__
#define __ALBITFIELDREADER_H__

#include "alinput.h"

struct bitfieldreader {
  int read;
  struct alinputstream *stream;
  int dataSize;
  int bitOffset;
  // stored as most significant bit first.
  unsigned int currentWord;
  int eof;
  int readbits; // last number of bits during read at eof to read...
};

struct bitfieldreader * new_fieldreader();

void fieldreader_init(struct bitfieldreader * this);

/* value returned is an int between 0 and 2^bits-1 */
int fieldreader_read( struct bitfieldreader * this, int bits );

void fieldreader_setinput( struct bitfieldreader * this, struct alinputstream * inputstream );

int bitfieldreader_is_eof(struct bitfieldreader * this);

int bitfieldreader_get_readbits(struct bitfieldreader * this);

#endif // #ifndef __ALBITFIELDREADER_H__
