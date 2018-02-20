#ifndef _ALBITFIELDWRITER_H_
#define _ALBITFIELDWRITER_H_

#include "aloutput.h"

struct bitfieldwriter {
  struct aloutputstream *stream;
  // in practice coded for bits per int == 32
  int dataSize;
  // offset within [0,dataSize[
  int bitOffset;
  // buffered bits
  // assume dataSize < bits_per_int  then can be stored as int
  unsigned int nextWord;  
};

// if dynamic allocation is needed.
struct bitfieldwriter * new_bitfieldwriter();

/** to release if allocated with new_bitfieldwriter
    remark this is a ** that will be set NULL after release */
void bitfieldwriter_release(struct bitfieldwriter ** pthis);

/** setup */
void bitfieldwriter_init(struct bitfieldwriter * this);

void bitfieldwriter_setoutputstream( struct bitfieldwriter * this, struct aloutputstream * outputstream);

/** write a value of bitsize in big endian */
void bitfieldwriter_write( struct bitfieldwriter * this, int value, int bitsize);

/*
  write/flush all current datas if any
  to have a clean word ready.
*/
void bitfieldwriter_padtoword( struct bitfieldwriter * this);

/** special flush at end 
write the minimal number of bytes given what remains in buffer 
*/
void bitfieldwriter_padtobyte( struct bitfieldwriter * this);

#endif // #ifndef _ALBITFIELDWRITER_H_
