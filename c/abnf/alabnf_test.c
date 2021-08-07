#include "alabnf_util.h"
#include "al_options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aldebug_output.h"
#include "aloutput.h"
#include "alabnf_dump.h"

void alabnf_normalize_name(char * in, char ** out)
{
  *out=in;
}

void usage()
{
  aldebug_printf(DBGSTREAM,"usage\n");
  aldebug_printf(DBGSTREAM,"infile=<abnf input file>");
}

int main(int argc, char ** argv)
{

  aldebug_start(NULL);
  
  struct al_options * options = al_options_create(argc,argv);
  aldatablock * infiledata = al_option_get(options,"infile");

  if ( infiledata != NULL )
    {
      printf("file to parse '" ALPASCALSTRFMT "'\n",
	     ALPASCALSTRARGS(infiledata->length,infiledata->data.charptr));
      FILE * file = fopen((char *)infiledata->data.ptr, "r");
      if ( file == NULL )
	{
	  aldebug_printf(DBGSTREAM,"[ERROR] fail to open '%s'\n",infiledata->data.charptr);
	}
      else
	{
	  struct alabnf * alabnf = alabnf_util_parse_abnf_file(file);
	  struct aloutputstream output;
	  {
	    aldebug_mute();
	    
	    aloutputstream_fd_init(&output, fileno(stdout));
	    alabnf_dump_rule(&output,&alabnf->root_rule);
	    
	    aldebug_unmute();
	  }
	}
    }
  else
    {
      usage();
      aldebug_printf(DBGSTREAM,"[ERROR] missing argument infile= file to parse.");
    }
    
  aldebug_printf(DBGSTREAM,"[TODO]\n");

  aldebug_end();
}
