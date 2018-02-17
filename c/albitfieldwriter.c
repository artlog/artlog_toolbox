#include <stdlib.h>
#include <limits.h>
#include "albitfieldwriter.h"
#include "altodo.h"
#include "aldebug.h"

int bits_per_int = sizeof(int) * CHAR_BIT;

void bitfieldwriter_newword(struct bitfieldwriter * this)
{
  if ( this->stream != NULL )
    {
      // do really write current word;
      aloutputstream_writeint32(this->stream,this->nextWord);
    }
  // reset nextWord
  this->bitOffset = 0;
  this->nextWord = 0;
}

struct bitfieldwriter * new_bitfieldwriter()
{
  return (struct bitfieldwriter *) calloc(1,sizeof(struct bitfieldwriter *));
}

void bitfieldwriter_release(struct bitfieldwriter ** pthis)
{
  if (*pthis != NULL)
    {
      free(*pthis);
      *pthis=NULL;
    }  
}

void bitfieldwriter_init(struct bitfieldwriter * this)
{
  this->dataSize=bits_per_int;
  this->bitOffset = 0;
  this->nextWord = 0;
  this->stream = NULL;
}

/** write at current offset
assume never write more than needed bits to complete a word 
assume meaningfull bits are aligned at right in least significant part */
void bitfieldwriter_internal_write(struct bitfieldwriter * this, int field, int bits)
{
  if ( bits == this->dataSize )
    { 
      // special case to keep sign
      this->nextWord = field; 
      this->bitOffset = 0;
    }
  else
    {
      field = field & ( 0x7FFFFFFF >> ( this->dataSize - 1 - bits));
      this->nextWord = this->nextWord | ( field << ( this->dataSize - bits - this->bitOffset));
      this->bitOffset = (this-> bitOffset + bits ) % this->dataSize; 
    }       
  // a new word is needed
  if ( this->bitOffset == 0 )
    {
      bitfieldwriter_newword(this);
    }
}

void bitfieldwriter_write(struct bitfieldwriter * this, int field, int bits)
{
  if ( this->dataSize >  bits_per_int )
    {
      aldebug_printf(NULL,"[FATAL] %i data size is more than int size\n", this->dataSize, bits_per_int );
    }
  if ( bits > this->dataSize )
    {
      aldebug_printf(NULL,"[FATAL] %i bits is more than data size\n", bits, this->dataSize );
    }

  // bits to fill a full dataSize word
  int bitstofill = this->dataSize - this->bitOffset;

  if ( bits > bitstofill )
  {
    // split current word in most significants bits / least significant bits.

    int remainingbits = bits - bitstofill;
    
    // most significant bits first, suppress least significant bits
    bitfieldwriter_internal_write( this, field >> remainingbits, bitstofill);

    // no need of newWord() previous write already done it

    // sent remaining least significant bits
    // keep only least significant bits last take care to not include bit of sign
    bitfieldwriter_internal_write( this, field & ( 0x7FFFFFFF >> ( bitstofill - 1)), remainingbits);
  }
  else
  {
    bitfieldwriter_internal_write( this, field, bits);
  }
}

void bitfieldwriter_padtoword(struct bitfieldwriter * this)
{
  if ( this->bitOffset != 0) {
       bitfieldwriter_newword(this);
  }
}

/* set OutputStream */
void bitfieldwriter_setoutputstream(struct bitfieldwriter * this, struct aloutputstream * output)
{
  this->stream = output;
}

