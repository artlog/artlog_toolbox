#include "alinput_util.h"
#include <stdlib.h>
#include <stdio.h>
#include "aldebug_output.h"

struct alinputstream * alinput_util_build_chain_stream_from_filenames(int filenames, int offset,char ** filename)
{
  struct alinputstream * container_inputstream = NULL;
  struct alinputstream * inputstream = NULL;
  FILE * file;

  for (int i=0; i <filenames; i++)
    {
      char * fname = filename[i+offset];
      file = fopen(fname,"r");
      if ( file != NULL )
	{
	  aldebug_printf(NULL,"[DEBUG] adding '%s' as input\n", fname);
	  inputstream = malloc(sizeof(*inputstream));
	  alinputstream_init(inputstream, fileno (file));
      
	  container_inputstream=alinputstream_create_chain(container_inputstream, inputstream);
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] adding '%s' fopen failed \n", fname);
	}
    }

  return container_inputstream;

}


int  alinput_util_destroy_chain_stream(struct alinputstream * head)
{
  // TODO implement method to dispose all after use ...
  return 0;
}
