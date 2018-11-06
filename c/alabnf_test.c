#include "alabnf.h"
#include "al_options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void alabnf_normalize_name(char * in, char ** out)
{
  *out=in;
}


int main(int argc, char ** argv)
{

  struct alabnf_sm   state_machine;

  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * infiledata = al_option_get(options,"infile");

  struct alinputstream main_inputstream;
  struct alinputstream * inputstream = NULL;

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
	  alinputstream_init(&main_inputstream, fileno (file));
	  inputstream=&main_inputstream;

	  alabnf_state_machine_init(&state_machine,inputstream);
	  alabnf_state_machine_run(&state_machine);
	  alabnf_state_machine_release(&state_machine);
	}
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] missing argument infile= file to parse.");
    }
    
  aldebug_printf(NULL,"[TODO]\n");
  
}
