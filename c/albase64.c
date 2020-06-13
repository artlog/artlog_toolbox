#include "albase64.h"
#include "aldebug.h"
#include "alinput.h"
#include "albitfieldreader.h"
#include <stdlib.h>

/* https://en.wikipedia.org/wiki/Base64#Base64_table */

char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char base64urlchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static char complement='=';

// read 6 bits
char albase64_6bitstochar(unsigned int inbits)
{
  return base64chars[inbits & 0x3f];
}

// read 6 bits
char albase64url_6bitstochar(unsigned int inbits)
{
  return base64urlchars[inbits & 0x3f];
}


int albase64func_frominput(char (*func_6bistochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output);

char * aleasybase64func(char (*func_6bistochar)(unsigned int),char * input, int length)
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

  {
    struct alinputstream inputstream;
    alinputstream_init(&inputstream,-1);
    alinputstream_setdatablock(&inputstream, &block,0);

    albase64func_frominput(func_6bistochar,&inputstream, &output);
  }

  // TODO detach output buffer to keep only malloc'ed buffer.
  char * buffer = output.buffer.data.charptr;
  return buffer;
}

char * aleasybase64(char * input, int length)
{
  aleasybase64func(albase64_6bitstochar,input,length);
}

char * aleasybase64url(char * input, int length)
{
  aleasybase64func(albase64url_6bitstochar,input,length);
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


int albase64func_frominput(char (*func_6bistochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output)
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
	b64char = (*func_6bistochar)(block);
	// addchar to output
	aloutputwritechar(output,b64char);
      }
  } while ( read == 6 );

  // pad input 6 bits * 4
  int pad = bits % 24;
  // 0,  17..23 => nothing.
  if ( ( pad > 0 ) && ( pad <=  16 ) )
    {
      // 9..16 => complement ('=')
      aloutputwritechar(output,complement);
      if ( pad <= 8 )
	{
	  // 1..8 =>  complement.complement ('==')
	  aloutputwritechar(output,complement);
	}
    }

  return 0;
}

int albase64_frominput(struct alinputstream * inputstream, struct aloutputstream * output)
{
  return albase64func_frominput(albase64_6bitstochar,inputstream,output);
}

int albase64url_frominput(struct alinputstream * inputstream, struct aloutputstream * output)
{
  return albase64func_frominput(albase64url_6bitstochar,inputstream,output);
}

int albase64(aldatablock * input, struct aloutputstream * output)
{

  struct alinputstream inputstream;
  alinputstream_init(&inputstream,-1);
  alinputstream_setdatablock(&inputstream, input,0);

  return albase64_frominput(&inputstream, output);
}
