#include "albase64.h"
#include "aldebug.h"
#include "alinput.h"
#include "albitfieldreader.h"
#include <stdlib.h>

/* https://en.wikipedia.org/wiki/Base64#Base64_table */

char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// read 6 bits
char albase64_6bitstochar(unsigned int inbits)
{
  return base64chars[inbits & 0x3f];
}

char * aleasybase64(char * input, int length)
{
  aldatablock block;

  block.data.charptr=input;
  block.length=length;
  block.type=ALTYPE_STR0;

  // TODO create a growable output buffer
  struct aloutputstream output;

  aldatablock outbuffer;
  int alloclength = length * 2;
  outbuffer.data.ptr = calloc(1,alloclength);
  outbuffer.length = alloclength;
  outbuffer.type = ALTYPE_STR0;

  aloutputstream_init_shared_buffer(&output, &outbuffer, 0);
  albase64(&block,&output);

  // TODO detach output buffer to keep only malloc'ed buffer.
  char * buffer = output.buffer.data.charptr;
  return buffer;
}

int aloutputwritechar(struct aloutputstream * output, char c)
{
  if ( output == NULL )
    {
      aldebug_printf(NULL,"%c",c);
    }
  else
    {
      aloutputstream_write_byte(output, (unsigned char) c);
    }
}

int albase64_frominput(  struct alinputstream * inputstream, struct aloutputstream * output)
{

  int bits = 0;
  int read = 0;
  char b64char =0;

  struct bitfieldreader bfreader;
  fieldreader_init(&bfreader);
  fieldreader_setinput(&bfreader,inputstream);

  unsigned int block;
  do {
    block = fieldreader_read( &bfreader, 6);
    if ( bitfieldreader_is_eof(&bfreader) )
      {
	read =  bitfieldreader_get_readbits(&bfreader);
      }
    else
      {
	read=6;
      }
    if ( read > 0 )
      {
	bits+=read;
	b64char = albase64_6bitstochar(block);
	// addchar to output
	aloutputwritechar(output,b64char);
      }
  } while ( read == 6 );

  // pad input 6 bits * 4
  int pad = bits % 24;
  // 0,  17..23 => nothing.
  if ( ( pad > 0 ) && ( pad <=  16 ) )
    {
      // 9..16 => '='
      aloutputwritechar(output,'=');
      if ( pad <= 8 )
	{
	  // 1..8 =>  '=='
	  aloutputwritechar(output,'=');
	}
    }

  return 0;
}

int albase64(aldatablock * input, struct aloutputstream * output)
{

  struct alinputstream inputstream;
  alinputstream_init(&inputstream,-1);
  alinputstream_setdatablock(&inputstream, input,0);

  return albase64_frominput(&inputstream, output);
}
