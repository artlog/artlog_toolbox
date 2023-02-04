#ifndef __ALBITFIELDREADER_H__
#define __ALBITFIELDREADER_H__

#include "alinput.h"

struct bitfieldreader {
  // cumulative read for this reader
  unsigned int currentRead;
  struct alinputstream *stream;
  int dataSize;
  // bit offset in currentRead
  int readOffset;
  // bit offset in currentWord
  int bitOffset;
  // stored as most significant bit first.
  unsigned int currentWord;
  int eof;
  // bit reads on source stream for currentWord
  int readbits; // last number of bits during read at eof to read...
};

struct bitfieldreader * new_fieldreader();

void fieldreader_init(struct bitfieldreader * this);

void fieldreader_setcharmode(struct bitfieldreader * this,unsigned int charbitsize);

/* 
   bits : number of bits we want to read, should be <= (sizeof(int) * 8 )
   value returned is an int between 0 and 2^bits-1 
   it always read all bits if possible.
   result is padded only when source stream reached eof  
   caller should check bitfieldread_is_eof and  bitfieldreader_get_readbits
   to know what part of result is valid
*/
int fieldreader_read( struct bitfieldreader * this, int bits );

/* set input stream source */
void fieldreader_setinput( struct bitfieldreader * this, struct alinputstream * inputstream );

/* reached end of stream */
int bitfieldreader_is_eof(struct bitfieldreader * this);

/* number of bits read
   relevant only if bitfieldreader_is_eof
   else is MUST be the requested number of bits
*/
int bitfieldreader_get_readbits(struct bitfieldreader * this);

#endif // #ifndef __ALBITFIELDREADER_H__
