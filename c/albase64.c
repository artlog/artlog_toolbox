#include "albase64.h"
#include "aldebug.h"
#include "alinput.h"
#include "albitfieldreader.h"
#include "albitfieldwriter.h"
#include <stdlib.h>
#include <stdio.h>

/* https://en.wikipedia.org/wiki/Base64#Base64_table */

char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char base64urlchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static char complement='=';

// 65 is an error, 64 is complement.
// fills up reverse table. charto6bits expected to be of length 256
void setup_charto6bits(char (*func_6bitstochar)(unsigned int), unsigned char *charto6bits)
{
  for (int i=0; i< 256; i++)
    {
      charto6bits[i]=65;
    }
  charto6bits[complement]=64;
  for (unsigned char i=0; i< 64; i++)
    {
      printf("%c",func_6bitstochar(i));
      charto6bits[func_6bitstochar(i)]=i;
    }
}
      

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

char * aleasybase64func(char (*func_6bitstochar)(unsigned int),char * input, int length)
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

    albase64func_frominput(func_6bitstochar,&inputstream, &output);
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


int albase64func_frominput(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output)
{
  int bits = 0;
  int read = 0;
  char b64char =0;

  struct bitfieldreader bfreader;
  fieldreader_init(&bfreader);
  // char mode
  fieldreader_setcharmode(&bfreader,8);
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
	b64char = (*func_6bitstochar)(block);
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


int albase64func_decode_frominput(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output)
{
  int bits = 0;
  int read = 0;
  unsigned char bitblock =0;
  unsigned char charto6bits[256];

  // build reverse table for char to 6bits
  setup_charto6bits(func_6bitstochar,charto6bits);

  /*
  struct bitfieldreader bfreader;
  fieldreader_init(&bfreader);
  fieldreader_setinput(&bfreader,inputstream);
  */
    
  struct bitfieldwriter bfoutput;
  bitfieldwriter_init(&bfoutput);
  // force 8bits char storage => buggy 
  // bfoutput.dataSize=8;
  bitfieldwriter_setoutputstream( &bfoutput,output);
    
  unsigned char c;
  do {
    c=alinputstream_readuchar(inputstream);
    if ( c==0)
      {
	read = 0;
      }
    else
      {
	read = 1;
      }
    
    /*
    block = fieldreader_read( &bfreader, 8);
    if ( bitfieldreader_is_eof(&bfreader) )
      {
	read =  bitfieldreader_get_readbits(&bfreader);
      }
    else
      {
	read=8;
      }
    */
    if ( read > 0 )
      {
	// does it trace ? where ?
	bitblock = charto6bits[c];
	// ... in debug mode only .. to check
	// aldebug_printf(DBGSTREAM,"%c%i.",c,bitblock);
	if ( bitblock < 64 )
	  {
	    // add 6 bits to output.
	    bitfieldwriter_write(&bfoutput, (unsigned int) bitblock, 6);
	  }
	else
	  if ( bitblock == 64 )
	    {
	      // complement case
	      // to check if it is right behavior : reset offset, don't write any remaing bits if any.
	      bfoutput.bitOffset=0;
	      read = 0;
	    }
	  else
	    {
	      // ERROR case
	      return 1;
	    }
      }
  } while ( read == 1 );

  
  bitfieldwriter_padtobyte(&bfoutput);
			   
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
