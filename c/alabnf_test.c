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

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * infiledata = al_option_get(options,"infile");

  if ( infiledata != NULL )
    {
      printf("file to parse '" ALPASCALSTRFMT "'\n",
	     ALPASCALSTRARGS(infiledata->length,infiledata->data.charptr));
      FILE * file = fopen((char *)infiledata->data.ptr, "r");
      if ( file == NULL )
	{
	  aldebug_printf(NULL,"[ERROR] fail to open '%s'\n",infiledata->data.charptr);
	}
      else
	{
	  struct alabnf * alabnf = alabnf_util_parse_abnf_file(file);
	  struct aloutputstream output;
	  aloutputstream_fd_init(&output, fileno(stderr));
	  alabnf_dump_rule(&output,&alabnf->root_rule);			   
	}
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] missing argument infile= file to parse.");
    }
    
  aldebug_printf(NULL,"[TODO]\n");
  
}
