#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "albitfieldreader.h"
#include "aldebug_output.h"

void tobitstring(unsigned int a,char buffer [33])
{
      for ( int i=0; i< 32;i++)
        {
          buffer[31-i] = '0' + (a & 1 );
          a >>= 1;
        }
      buffer[32] = 0;
}

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
  this->lastreadbits=0;
  this->eof=0;
  this->currentRead=0;
  this->bitOffset=0;
}

void fieldreader_setcharmode(struct bitfieldreader * this,unsigned int charbitsize)
{
  this->dataSize = charbitsize;
}

void fieldreader_free(struct bitfieldreader * this)
{
  free(this);
}

// fully reset currentWord,bitOffset and readbits
unsigned int fieldreader_nextword(struct bitfieldreader * this)
{
  unsigned int field = 0;
  if ( this->dataSize == 8 )
    {
      // assume char mode
      field = (unsigned int) alinputstream_readuchar(this->stream);
    }
  else
    {
      field = alinputstream_readuint32(this->stream);
    }

#ifdef DEBUG
	    {
	      char fieldstr[33];
	      tobitstring(field,fieldstr);
	      aldebug_printf(DBGSTREAM,"%s:%i source EOF currentWord %08x %s currentOffset, %i readbits %i\n",
			     __FUNCTION__, __LINE__,
			     field, fieldstr, this->bitOffset,  this->readbits);
	    }
#endif

  if ( this->stream->eof == 1 )
    {
#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"source EOF currentWord %08x, currentOffset, %i readbits %i\n", this->currentWord, this->bitOffset,  this->readbits);
	    }
#endif

      // input stream read bits is always aligned on CHAR_BITS
      int bits = alinputstream_get_readbits(this->stream);
      if ( bits > 0 )
	{
	  // field is aligned most signifiant bits first
	  this->currentWord = field;
	  // skip missing bits ( align bitOffset with readbits ) => this is the issue !?
	  this->readbits = bits;
	  this->bitOffset = this->dataSize-bits;
#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"EOF readbits %i\n",  this->readbits);
	    }
#endif
	}
      else
	{
#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"EOF readbits %i bits \n",  this->readbits, bits);
	    }
#endif
	  this->readbits=0;
	  this->currentWord = 0;
	}
    }
  else
    {
      // read next word
      this->currentWord = field;
      this->readbits=this->dataSize;
      this->bitOffset = 0;
    }

#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"Read word %08x  \n",  field);
	    }
#endif

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

void bitfieldreader_set_eof(struct bitfieldreader * this, int lastreadbits)
{
  if ( this->eof == 1 )
    {
      aldebug_printf(DBGSTREAM,"[ERROR] eof called twice %i source eof %i (readbits %i) \n",  this->eof, this->stream->eof, this->readbits);
      return;
    }
  this->eof=1;
  this->lastreadbits=lastreadbits;
}

unsigned int bitfieldreader_internal_read( struct bitfieldreader * this, int bits )
{

#ifdef DEBUG
	    {
	      char buffer [33];
	      tobitstring(this->currentWord,buffer);
	      aldebug_printf(DBGSTREAM,"enter %s:%i reading bit on offset %i bits %i currentWord %08x %s\n",__FUNCTION__,__LINE__, this->bitOffset, bits,this->currentWord, buffer);
	    }
#endif

  if ( this->eof )
    {      
      aldebug_printf(DBGSTREAM,"ERROR reading on a bitstream that reached eof already\n");
      return 0;
    }

  // source aleady met eof, should consume current word
  if ( this->stream->eof )
    {
      // consume some extra unread bits.
      if ( bits < this->readbits )
	{
	  this->lastreadbits = bits;
	  this->readbits -= bits;
	  if ( this->bitOffset == 0 )
	    {
#ifdef DEBUG
	    {
	      char buffer [33];
	      tobitstring(this->currentWord,buffer);
	      aldebug_printf(DBGSTREAM,"%s reading bit on offset %i bits %i currentWord %08x %s\n",__FUNCTION__, this->bitOffset, bits,this->currentWord, buffer);
	    }
#endif
	      
	      return this->currentWord;
	    }	  
	  // we have read needed bits but the are not aligned correctly
	  {
	    unsigned int field = 0;
#ifdef DEBUG
	    {
	      char buffer [33];
	      tobitstring(this->currentWord,buffer);
	      aldebug_printf(DBGSTREAM,"%s:%i reading bit on offset %i bits %i currentWord %08x %s\n",__FUNCTION__,__LINE__, this->bitOffset, bits,this->currentWord, buffer);	      
	    }
#endif
	    
	    field = this->currentWord >> ( this->dataSize - bits );
	    // most significant bits are read then removed them from currentWord
	    this->currentWord <<= bits;
	    this->currentRead <<= bits;
	    this->bitOffset = ( this->bitOffset + bits ) % this->dataSize;

	    return field;
	  }

	}
      else if ( bits == this->readbits )
	{
	  bitfieldreader_set_eof(this, this->readbits);
	}
      else
	{
	  // want to consume more bits than remaining
	  bits = this->readbits;
	  bitfieldreader_set_eof(this, this->readbits);
	  // all read bits were consumed
	  this->readbits = 0;
	}
    }

  if ( bits == 0 )
    {
#ifdef DEBUG
      aldebug_printf(DBGSTREAM,"ERROR reading 0 bits on a bitstream\n");
#endif
      return 0;
    }

#ifdef DEBUG
  aldebug_printf(DBGSTREAM,"%s:%i reading bitOffset %i readbits %i currentWord %08x\n",__FUNCTION__,__LINE__, this->bitOffset, this->readbits, this->currentWord);
#endif

  // what we read so far...
  unsigned int field = this->currentWord;

  // a new word is needed
  if ( this->bitOffset == 0 ) {
    fieldreader_nextword(this);
    this->currentRead = this->currentWord;

#ifdef DEBUG
    char buffer [33];
    tobitstring(this->currentWord,buffer);
    aldebug_printf(DBGSTREAM,"%s:%i reading bitOffset %i readbits %i currentWord %08x %s\n",__FUNCTION__,__LINE__, this->bitOffset, this->readbits, this->currentWord, buffer);
#endif

    if ( this->stream->eof )
      {
	if ( bits <= this->readbits )
	  {
	    this->readbits -= bits;
	  }
	else
	  {
	    bits = this->readbits;
	    bitfieldreader_set_eof(this,this->readbits);
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
    this->currentRead <<= bits;
    this->bitOffset = ( this->bitOffset + bits ) % this->dataSize;
  }

  return field;
}

// assumption is done here that bits can't be > 2 * dataSize
// but this might be wrong ? nope int return so there is an obvious limit
int fieldreader_read( struct bitfieldreader * this, int bits )
{
  unsigned int field = 0;
  unsigned int head = 0;

#ifdef DEBUG
  aldebug_printf(DBGSTREAM,"enter fieldreader_read %i bitOffset %i dataSize %i currentWord %08x\n", bits, this->bitOffset, this->dataSize, this->currentWord);
#endif

  if ( this->eof )
    {
      return 0;
    }

  // if more bits needed than word currently used.
  if ( ( bits  + this->bitOffset ) > this->dataSize )
    {
      int bitsize = this->dataSize - this->bitOffset;
      head = bitfieldreader_internal_read( this, bitsize);

      // actualy do padding with 0.
      int missingbits = bits - bitsize;

      if ( this->eof )
	{
	  field = (head >> this->bitOffset) << missingbits;
	  //#ifdef DEBUG
	  {
	        char fieldstr [33];
		tobitstring(field,fieldstr);
		char headstr [33];
		tobitstring(head,headstr);
		aldebug_printf(DBGSTREAM,"%s:%i field %08x head %08x bits %i bitsize %i readbits %i bitOffset %i missingbits %i %s %s\n", __FUNCTION__, __LINE__, field, head, bits, bitsize, this->readbits, this->bitOffset, missingbits, fieldstr, headstr);
	  }
	  //#endif
	  // return bit read without padding
	  this->readbits=bitsize;
	  return field;
	}

      if ( bits == bitsize )
	{
#ifdef DEBUG
	  {
	    aldebug_printf(DBGSTREAM,"%s:%i head %08x bits %i  bitsize %i\n", __FUNCTION__, __LINE__ ,head, bits, bitsize);
	  }
#endif

	  return head;
	}

      if ( bits < bitsize )
	{
#ifdef DEBUG
	  {
	    aldebug_printf(DBGSTREAM,"head %08x bits %i  bitsize %i\n", head, bits, bitsize);
	  }
#endif
	  return head;
	}

      // not this->eof and not bits == bitsize

      // more significant bits in first word, least in last
      int shiftbits = missingbits;


      // current word had entirely been consumed, need more to complete request.
      // not fully true
      // current source word can be fully read, but not fully consumed.

#ifdef DEBUG
      {
	aldebug_printf(DBGSTREAM,"%s:%i need more to complete request.\n", __FUNCTION__, __LINE__);
      }
#endif

      field = bitfieldreader_internal_read( this, missingbits);

      if ( this->eof )
	{
	  // all was read.
	  // less than bits, bitsize + what was read above
	  if ( bitsize + this->lastreadbits <= bits )
	    {
	      this->lastreadbits = bitsize + this->lastreadbits;
	    }
	  else
	    {
	      aldebug_printf(DBGSTREAM,"[ERROR] %s:%i last read bits %i unexpected here .\n", __FUNCTION__, __LINE__,this->lastreadbits);
	    }
	  // because debugged this way ...
	  // shiftbits = this->dataSize - bits;
	  shiftbits = 0;
	}

#ifdef DEBUG
	{
	  aldebug_printf(DBGSTREAM,"field %08x bits %i  bitsize %i shiftbits %i\n", field, bits, bitsize, shiftbits);
	}
#endif

      // reconstruct all
      // more significant bits in first word, least in last
      field = field | ( head << shiftbits);

#ifdef DEBUG
	{
	  aldebug_printf(DBGSTREAM,"head %08x tail %08x\n", head, field);
	}
#endif
    }
  else {
    field = bitfieldreader_internal_read(this,bits);
#ifdef DEBUG
	{
	  aldebug_printf(DBGSTREAM,"%s:%i field %08x\n", __FUNCTION__, __LINE__, field);
	}
#endif
  }

#ifdef DEBUG
	{
	  aldebug_printf(DBGSTREAM,"exit fieldreader_read %i bitOffset %i dataSize %i\n", bits, this->bitOffset, this->dataSize);
	}
#endif

  return field;
}

int bitfieldreader_is_eof(struct bitfieldreader * this)
{
#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"EOF ? %i source eof %i (readbits %i) \n",  this->eof, this->stream->eof, this->readbits);
	    }
#endif

  return this->eof;
}

int bitfieldreader_get_readbits(struct bitfieldreader * this)
{
#ifdef DEBUG
	    {
	      aldebug_printf(DBGSTREAM,"get last read bits %i %i\n", this->lastreadbits, this->readbits);
	    }
#endif

  return this->lastreadbits;
}
