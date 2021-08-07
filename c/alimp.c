#include "albitfieldreader.h"
#include "albitfieldwriter.h"
#include "al_options.h"
#include "aldebug.h"

#include <stdlib.h>
#include <stdio.h>
#include "aldebug_output.h"
#include "alinput_file.h"

static const char * INFILESTR0="infile";

unsigned char charblock_global[1024];


struct alimp_info {
  int todo;
  struct alinputstream * inputstream;
};

void usage()
{
  aldebug_printf(DBGSTREAM,"try something on .imp leica format\n");
  aldebug_printf(DBGSTREAM,"infile: file to read\n");
}


struct alinputstream * getinputstream(struct al_options * options)
{
  struct alinputstream * inputstream = NULL;
  aldatablock * opt1 = al_option_get(options,INFILESTR0);
  if ( opt1 != NULL )
    {
      char * filename = opt1->data.charptr;
      inputstream = malloc(sizeof(*inputstream));
      if ( alinput_file_open_init(inputstream,filename) != AL_EC_OK )
	{
	  aldebug_printf(DBGSTREAM,"[ERROR] no such '%s' file\n", filename);
	  free(inputstream);
	  inputstream = NULL;
	}
    }  
  return inputstream;
}

void alimp_info_init(struct alimp_info * alimp_info,  struct alinputstream * inputstream)
{
  alimp_info->inputstream = inputstream;
}

void alimp_dump_hexline(  struct alinputstream * inputstream, int blocksize, int charsperline, int textmul)
{
  unsigned char c;
  if ( blocksize > 1024 )
    {
      blocksize = 1024;
      aldebug_printf(DBGSTREAM,"[FATAL] call paramter exceed internal buffer size %i\n", blocksize);
    }
  int cpl = charsperline;
  int l = 0;
  for (int i =0; i<blocksize; i++)
    {
      if ( (i % cpl) == 0 )
	{
	  l++;
	  printf("\n%02i:",l);
	}
      c = alinputstream_readuchar(inputstream);
      charblock_global[i] = c;
      printf("%02x ",c);
    }
  printf("\n");
  cpl = charsperline * textmul;
  for (int i =0; i<1024; i++)
    {
      c=charblock_global[i];
      printf("%c",((c >= 32) && (c<128)) ? c :  (( c == 0 ) ? '.' :'-' ) );
      if ( (i % cpl) == (cpl - 1) )
	{
	  printf("\n");
	}
    }
}

void alimp_dump_1k_hex(  struct alinputstream * inputstream )
{
  alimp_dump_hexline(inputstream, 1024, 32, 1);
}



void alimp_read_header(struct alimp_info * alimp_info)
{
  struct alinputstream * inputstream = alimp_info->inputstream;
  
  alimp_dump_hexline(inputstream,16*32,32,1);
  alimp_dump_hexline(inputstream,7*32,32,1);
  alimp_dump_hexline(inputstream,9*32,32,1);
  for ( int b=0; b < 8;b++)
    {
      printf("\n1k block #%i\n",(b+1));
      alimp_dump_1k_hex(inputstream);
    }
  for ( int b=0; b < 30;b++)
    {
      printf("\n780b block #%i\n",(b+1));
      // 78 = 2 * 39
      alimp_dump_hexline(inputstream,780,39,2);
    }
  for ( int b=0; b < 100;b++)
    {
      printf("\n1k block #%i\n",(b+1));
      alimp_dump_1k_hex(inputstream);
    }
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  al_options_set_debug(options,0);
  usage();

  struct alinputstream * inputstream =  getinputstream(options);
  struct alimp_info alimp_info_static;
  struct alimp_info * alimp_info = &alimp_info_static;

  if (inputstream != NULL )
    {
      alimp_info_init(alimp_info,inputstream);
      alimp_read_header(alimp_info);
    }

  al_options_release(options);
}
