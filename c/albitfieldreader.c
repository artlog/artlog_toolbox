#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "albitfieldreader.h"

struct bitfieldreader * new_fieldreader()
{
  return calloc(1,sizeof(struct bitfieldreader));
}

void fieldreader_init(struct bitfieldreader * this)
{
  // in bits size read
  this->dataSize=32;
}

void fieldreader_free(struct bitfieldreader * this)
{
  free(this);
}

void fieldreader_nextword(struct bitfieldreader * this)
{
  // read next word
  this->currentWord= alinputstream_readuint32(this->stream);
  this->bitOffset = 0;
}

/*
  go to next entire word.
*/
void fieldreader_padtoword(struct bitfieldreader * this)
{
  if ( this->bitOffset != 0) {
    fieldreader_nextword(this);
  }
}

void fieldreader_setinput( struct bitfieldreader * this, struct alinputstream * inputstream )
{
  this->stream=inputstream;
}

int fieldreader_read( struct bitfieldreader * this, int bits )
{
  int field = 0;
  int head = 0;

  // a new word is needed
  if ( this->bitOffset == 0 ) {
    if ( this->stream->eof )
      {
	return field;
      }
    fieldreader_nextword(this);    
  }
  // if more bits needed that word currently used.
  if ( ( bits  + this->bitOffset ) > this->dataSize)
    {
      // warning recursive call ( not terminal )
      int bitsize = this->dataSize - this->bitOffset;
      head = fieldreader_read( this, bitsize);
      // current word had entirely been read, need a new one
      // don't do that... nextword() will be done by next read...
      field = fieldreader_read( this, bits - bitsize);
      // reconstruct all
      // more significant bits in first word, least in last
      field = field | ( head << (bits - bitsize));
    }
  else {
    // terminal part do the job
    if ( bits == this->dataSize) {
      // special case to keep sign
      field = this->currentWord;
      this->currentWord = 0;
      this->bitOffset = 0;
    }
    else {       
      field = ( this->currentWord >> ( this->dataSize - bits) )
	& ( 0x7fffffff >> ( this->dataSize - 1 - bits) );
      // bits are read then remove them from currentWord
      this->currentWord <<= bits;
      this->bitOffset = ( this->bitOffset + bits ) % this->dataSize;
    }
  }  
  return field;  
}

