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
  this->currentWord=0;
  this->dataSize=32;
  this->readbits=0;
  this->eof=0;
}

void fieldreader_free(struct bitfieldreader * this)
{
  free(this);
}

unsigned int fieldreader_nextword(struct bitfieldreader * this)
{
  unsigned int field  = alinputstream_readuint32(this->stream);
  if ( this->stream->eof == 1 )
    {
      int bits = alinputstream_get_readbits(this->stream);
      if ( bits > 0 )
	{
	  // field is aligned most signifiant bits first
	  // this->currentWord = field >> (this->dataSize-bits);
	  this->currentWord = field;
	  // reached eof while reading word, then not fully read word.
	  this->readbits=bits;
	  aldebug_printf(NULL,"EOF readbits %i\n",  this->readbits);
	}
    }
  else
    {
      // read next word
      this->currentWord = field;
    }
  this->bitOffset = 0;
  return field;
}

/*
  go to next entire word.
*/
void fieldreader_padtoword(struct bitfieldreader * this)
{
  if ( this->bitOffset != 0) {
    // fixme might fail if eof is reached unaligned
    fieldreader_nextword(this);
  }
}

void fieldreader_setinput( struct bitfieldreader * this, struct alinputstream * inputstream )
{
  this->stream=inputstream;
}

unsigned int bitfieldreader_internal_read( struct bitfieldreader * this, int bits )
{

  if ( this->eof )
    {
      return 0;
    }
  
  if ( this->stream->eof )
    {
      // consume some extra unread bits.
      if ( bits <= this->readbits )
	{
	  this->readbits -= bits;
	}
      else
	{
	  bits = this->readbits;
	  this->eof=1;
	}

      if ( this->bitOffset == 0 )
	{
	  return this->currentWord;
	}
      
    }

  if ( bits == 0 )
    {
      return 0;
    }

  unsigned int field = 0;
  
  // a new word is needed
  if ( this->bitOffset == 0 ) {
    fieldreader_nextword(this);
    if ( this->stream->eof )
      {
	if ( bits <= this->readbits )
	  {
	    this->readbits -= bits;
	  }
	else
	  {
	    bits = this->readbits;
	    this->eof=1;
	  }	
      }
  }

  // terminal part do the job
  if ( bits == this->dataSize ) {
    // special case to keep sign
    field = this->currentWord;
    this->currentWord = 0;
    this->bitOffset = 0;
  }
  else {       
    field = this->currentWord >> ( this->dataSize - bits);
    // most significant bits are read then removed them from currentWord
    this->currentWord <<= bits;
    this->bitOffset = ( this->bitOffset + bits ) % this->dataSize;
  }

  return field;
}

int fieldreader_read( struct bitfieldreader * this, int bits )
{
  unsigned int field = 0;
  unsigned int head = 0;

  if ( this->eof )
    {
      return 0;
    }

  // if more bits needed that word currently used.
  if ( ( bits  + this->bitOffset ) > this->dataSize)
    {
      int bitsize = this->dataSize - this->bitOffset;
      head = bitfieldreader_internal_read( this, bitsize);

      if ( this->eof )
	{
	  return head;
	}
      // current word had entirely been read, need a new one
      // don't do that... nextword() will be done by next read...
      field = bitfieldreader_internal_read( this, bits - bitsize);

      if ( this->eof )
	{	  
	  return head;
	}      

      aldebug_printf(NULL,"head %08x tail %08x\n", head, field);
      // reconstruct all
      // more significant bits in first word, least in last
      field = field | ( head << (bits - bitsize));

      return field;  
    }
  else {
    return bitfieldreader_internal_read(this,bits);
  }  
}

int bitfieldreader_is_eof(struct bitfieldreader * this)
{
  return this->eof;
}

int bitfieldreader_get_readbits(struct bitfieldreader * this)
{
  return this->readbits;
}
