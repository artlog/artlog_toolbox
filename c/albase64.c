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
#define _6BITSMASK 0x3f

/* TODO keep context for reuse */
struct albase64_context {
  int flags;
  char base64chars[65];
  unsigned char charto6bits[128];
};
  
// 65 is an error, 64 is complement.
// fills up reverse table. charto6bits expected to be of length 128
// This is US ASCII, so should not be > 127 ?
void setup_charto6bits(char (*func_6bitstochar)(unsigned int), unsigned char *charto6bits)
{
  for (int i=0; i< 128; i++)
    {
      charto6bits[i]=65;
    }
  charto6bits[complement]=64;
  char c = 0;
  for (unsigned char i=0; i<= _6BITSMASK; i++)
    {
      c=func_6bitstochar(i);
#ifdef DEBUG_BASE64
      printf("%c",c);
#endif
      if ( c >= 0 )
	{
	  charto6bits[c]=i;
	}
      else
	{
	  // this is a problem, characters should be US ASCII
	}
    }
}

// read 6 least significant bits from inbits and return char
char albase64_6bitstochar(unsigned int inbits)
{
  return base64chars[inbits & _6BITSMASK];
}

// read 6 bits
char albase64url_6bitstochar(unsigned int inbits)
{
  return base64urlchars[inbits & _6BITSMASK];
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
  int bitreadden = 0;
  char b64char =0;

  struct bitfieldreader bfreader;
  fieldreader_init(&bfreader);
  // char mode, WHY ? seems buggy
  // fieldreader_setcharmode(&bfreader,8);
  fieldreader_setinput(&bfreader,inputstream);

  unsigned int block;
  int iseof = 0;
  do {
    block = fieldreader_read( &bfreader, 6);
    iseof = bitfieldreader_is_eof(&bfreader);
    if ( iseof )
      {
	bitreadden = bitfieldreader_get_readbits(&bfreader);
#ifdef DEBUG_BASE64	
	aldebug_printf(DBGSTREAM,"*** eof bitreadden %i block %08x ***\n",bitreadden, block);
#endif	
      }
    else
      {
	bitreadden=6;
      }
    if ( bitreadden > 0 )
      {
	bits+=bitreadden;
	b64char = (*func_6bitstochar)(block);
#ifdef DEBUG_BASE64
	aldebug_printf(DBGSTREAM,"b64char '%c'(%x) block %08x bit readden %i\n",b64char,b64char,block,bitreadden);
#endif
	// addchar to output
	aloutputwritechar(output,b64char);
      }
#ifdef DEBUG_BASE64
    else
      {
	aldebug_printf(DBGSTREAM,"block %08x bit readden %i\n",block,bitreadden);
      }
#endif
  } while ( ( bitreadden == 6 ) && ( ! iseof) );

  // pad input 6 bits * 4
  int pad = bits % 24;

#ifdef DEBUG_BASE64
  aldebug_printf(DBGSTREAM,"pad %i bits %i\n",pad, bits);
#endif
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


int albase64func_decode_tobitfieldwriter(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct bitfieldwriter * bfoutput_p)
{
  int bits = 0;
  // bytes
  int read = 0;
  unsigned char bitblock =0;
  unsigned char charto6bits[128];

  // build reverse table for char to 6bits
  setup_charto6bits(func_6bitstochar,charto6bits);

#ifdef USEBITFIELDREADER
  struct bitfieldreader bfreader;
  fieldreader_init(&bfreader);
  fieldreader_setinput(&bfreader,inputstream);
#endif
    
  unsigned char c;
  do {

#ifdef USEBITFIELDREADER
    block = fieldreader_read( &bfreader, 8);
    if ( bitfieldreader_is_eof(&bfreader) )
      {
	read =  bitfieldreader_get_readbits(&bfreader);
      }
    else
      {
	read=8;
      }
#else
    c = alinputstream_readuchar(inputstream);
    read = (c==0) ? 0 : 1;
#endif
    
    if ( read > 0 )
      {
	bitblock = charto6bits[c];
	// ... in debug mode only .. to check
#ifdef DEBUG_BASE64
	aldebug_printf(DBGSTREAM,"%c%i.",c,bitblock);
#endif
	if ( bitblock < 64 )
	  {
	    // add 6 bits to output.
	    bitfieldwriter_write(bfoutput_p, (unsigned int) bitblock, 6);
	  }
	else
	  if ( bitblock == 64 )
	    {
	      // complement case
	      // should flush bitfieldwriter, rely on bitfielwriter_padtobyte
	      read = 0;
	    }
	  else
	    {
	      // ERROR case
	      return 1;
	    }
      }
  } while ( read == 1 );

#ifdef DEBUG_BASE64
  aldebug_printf(DBGSTREAM," final padding \n");
#endif

  bitfieldwriter_padtobyte(bfoutput_p);

  return 0;
}

int albase64func_decode_frominput(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output)
{
    
  struct bitfieldwriter bfoutput;
  bitfieldwriter_init(&bfoutput);
  // force 8bits char storage => buggy ??? WHY ???
  // bfoutput.dataSize=8;
  bitfieldwriter_setoutputstream( &bfoutput,output);

  return albase64func_decode_tobitfieldwriter(func_6bitstochar , inputstream, &bfoutput);
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
