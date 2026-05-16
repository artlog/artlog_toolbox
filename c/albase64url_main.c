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
  aldebug_printf(DBGSTREAM,"\
program <name of file to get base64 url> (<debug>)\n\
-h this help\n\
-d decode\n\
-e encode\n\
-x read hex\n\
-u use base64 url encoding\n\
-a will use following argument as a direct value\n\
in=<input filnename>, use stdin if not set\n\
out=<output filename>, use stdout if not set\n\
");
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
  aldatablock * embeded_value = al_option_get(options,"value");

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

  struct albase64_context * albase64_context_p = base64url ? &ALBASE64_CONTEXT_URL : &ALBASE64_CONTEXT_DEFAULT;

  if ( help == 0 )
    {
      struct aloutputstream output;
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

      int encode_decode = 0;
      FILE * f = NULL;
      struct alinputstream input;
      if ( embeded_value != NULL ) {
	aldebug_printf(DBGSTREAM,"[INFO] direct input\n");
	alinputstream_init(&input,-1);
	embeded_value->type=ALTYPE_STR0;
	alinputstream_setdatablock(&input, embeded_value,0);
	encode_decode = 1;
      }
      else {
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
	    alinputstream_init(&input, fileno(f));
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
		fclose(f);
		f=NULL;
		encode_decode = 0;
	      }
	    else
	      {
		encode_decode = 1;
	      }
	  }
	else
	  {
	    aldebug_printf(DBGSTREAM,"[ERROR] failed to open file '%s'\n", filename );
	  }
      }

      if ( encode_decode == 1 )
	{
	  if ( encode )
	    {
	      albase64_encode(albase64_context_p,&input,&output);
	    }
	  else
	    {
	      albase64_decode(albase64_context_p,&input,&output);
	    }
	}
      if ( f != NULL )
	{
	  fclose(f);
	}
      // todo close output too...
    }
  else
    {
      usage();
    }
  aldebug_end();
}
