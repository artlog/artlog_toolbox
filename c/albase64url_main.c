#include "alcryptohash_tool.h"

#include "altodo.h"
#include "aldebug.h"
#include "albase64.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "aldebug_output.h"
#include "aloutput.h"
#include "al_options.h"
#include "al_options_output.h"

void usage()
{
  aldebug_printf(DBGSTREAM,"program <name of file to get base64 url> (<debug>)\n");
  aldebug_printf(DBGSTREAM,"-h this help\n");
  aldebug_printf(DBGSTREAM,"-d decode\n");
  aldebug_printf(DBGSTREAM,"-e encode\n");
  aldebug_printf(DBGSTREAM,"-x read hex\n");
  aldebug_printf(DBGSTREAM,"-u use base64 url encoding\n");
  aldebug_printf(DBGSTREAM,"in=<input filnename>, use stdin if not set\n");
  aldebug_printf(DBGSTREAM,"out=<output filename>, use stdout if not set\n");
}

void test_base64(char * text)
{
  char * result = NULL;

  result = aleasybase64url(text,strlen(text));

  if ( result != NULL )
    {
      printf("base64('%s')='%s'\n",text,result);
      free(result);
    }
}

int main(int argc, char ** argv)
{
  aldebug_start(NULL);

  char * filename = NULL;
  char * out_filename = NULL;
  
  struct al_options * options = al_options_create(argc,argv);

  int encode = (al_option_get(options,"e") == NULL) ? 0 : 1;
  int base64url = (al_option_get(options,"u") == NULL) ? 0 : 1;
  int hexin = (al_option_get(options,"x") == NULL) ? 0 : 1;
  int help = (al_option_get(options,"h") == NULL) ? 0 : 1;
  
  aldatablock * in_filename_value = al_option_get(options,"in");
  aldatablock * out_filename_value = al_option_get(options,"out");
  
  if ( in_filename_value != NULL )
    {
      filename = in_filename_value->data.charptr;
    }

  if ( out_filename_value != NULL )
    {
      out_filename = out_filename_value->data.charptr;
    }

  // don't set debug to options
  al_options_set_debug(options,0);

  char (*func_6bits_to_char) (unsigned int) = base64url ? albase64url_6bitstochar : albase64_6bitstochar;

  if ( help == 0 )
    {
      FILE * f = NULL;
      if ( (filename != NULL) && (strlen(filename) > 0))
	{
	  f = fopen(filename,"r");
        }
      else
        {
          f = stdin;
        }      
      if ( f != NULL )
        {
          struct alinputstream input;
          struct aloutputstream output;
          alinputstream_init(&input, fileno(f));
          FILE * fout = NULL;
          if ( out_filename != NULL )
            {
              fout=fopen(out_filename,"w");
              if ( fout != NULL )
                {
                  aloutputstream_fd_init(&output, fileno(fout));
                }
              else
                {
                  aldebug_printf(DBGSTREAM,"[ERROR] failed to create out file '%s'\n", out_filename );
                  exit(1);
                }
            }
          else
            {
              aloutputstream_fd_init(&output, fileno(stdout));
            }
          if ( hexin )
            {
              aldebug_printf(DBGSTREAM,"[INFO] hex input\n");
                aldatablock block;
                int length=1024;
                char * buffer = calloc(1,length);
                block.data.charptr=buffer;
                block.length=length;
                block.type=ALTYPE_STR0;
                int bytesread = 0;
                alinputstream_readhex_stream(&input, &block, &bytesread);
                block.length=bytesread;
                albase64(&block, &output);
                free(buffer);
            }
          else if ( encode )
            {
              albase64func_frominput(func_6bits_to_char,&input,&output);
            }
          else
            {
              albase64func_decode_frominput(func_6bits_to_char,&input,&output);
            }
          fclose(f);
          // todo close output too...
        }
      else
        {
          aldebug_printf(DBGSTREAM,"[ERROR] failed to open file '%s'\n", filename );
        }
    }
  else
    {
      usage();
    }
  aldebug_end();
}
