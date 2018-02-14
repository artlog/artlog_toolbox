#ifndef _ALBITFIELDWRITER_H_
#define _ALBITFIELDWRITER_H_

#include "aloutput.h"

struct bitfieldwriter {
  struct aloutputstream *stream;
  int dataSize;
  int bitOffset;
  int nextWord;  
};

struct bitfieldwriter * new_bitfieldwriter();

void bitfieldwriter_init(struct bitfieldwriter * this);

void bitfieldwriter_setoutputstream( struct bitfieldwriter * this, struct aloutputstream * outputstream);

void bitfieldwriter_write( struct bitfieldwriter * this, int value, int bitsize);

void bitfieldwriter_padtoword( struct bitfieldwriter * this);

#endif // #ifndef _ALBITFIELDWRITER_H_
