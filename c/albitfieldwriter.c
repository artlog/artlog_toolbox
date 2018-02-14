#include <stdlib.h>
#include "albitfieldwriter.h"

void bitfieldwriter_newword(struct bitfieldwriter * this);

struct bitfieldwriter * new_bitfieldwriter()
{
  return (struct bitfieldwriter *) calloc(1,sizeof(struct bitfieldwriter *));
}

void bitfieldwriter_init(struct bitfieldwriter * this)
{
  this->dataSize=32;
}
  
void bitfieldwriter_write(struct bitfieldwriter * this, int field, int bits)
{
     // if more bits needed that word currently used.
     if ( bits  + this->bitOffset > this->dataSize) {
       // warning recursive call ( not terminal )
       // fill and write current word
       int bitsize = this->dataSize - this->bitOffset;
       int head;
       // most significant bits first
       // suppress at left least significant bits
       head = field >> (bits - bitsize);
       bitfieldwriter_write( this, head, bitsize);
       // no need of newWord() previous write already done it
       // least significant bits last
       field = field & ( 0x7FFFFFFF >> ( bitsize - 1));
       bitfieldwriter_write( this, field, bits - bitsize);
     }
     else {         
       if ( bits == this->dataSize )
       {
         // special case to keep sign
         this->nextWord = field; 
         this->bitOffset = 0;
       }
       else
       {
         // keep only meaning part of field
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
   }


/*
     * seems to works correctly as DataOutputStream.writeInt()

void writeInt( int i)
    {
	long j = i;
	byte bytes[]=new byte[4];
	bytes[0] = (byte) ((j & 0xFF000000) >> 24);
	bytes[1] = (byte) ((j & 0x00FF0000) >> 16);
	bytes[2] = (byte) ((j & 0x0000FF00) >> 8);
	bytes[3] = (byte) (j & 0x000000FF);
	outputStream.write( bytes, 0, 4);
    }
     */

void bitfieldwriter_newword(struct bitfieldwriter * this)
{
  // do really write current word;
  //writeInt( nextWord);
  aloutputstream_writeint32(this->stream,this->nextWord);
  // reset nextWord
  this->bitOffset = 0;
  this->nextWord = 0;
}
   /*
    write all current datas if any
    to have a clean word ready.
    */
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

