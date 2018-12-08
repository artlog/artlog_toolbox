#include "alabnf_util.h"
#include <stdlib.h>

struct alabnf * alabnf_util_parse_abnf_file(FILE * file)
{
  struct alabnf_sm   state_machine;
  struct alinputstream main_inputstream;
  struct alinputstream * inputstream = NULL;
  
  alinputstream_init(&main_inputstream, fileno (file));
  inputstream=&main_inputstream;
  
  alabnf_state_machine_init(&state_machine,inputstream);
  alabnf_state_machine_run(&state_machine);
  alabnf_state_machine_release(&state_machine);

  return alabnf_state_machine_generated(&state_machine);
}

// TODO provide a method to dispose all after use ... 
struct alabnf * alabnf_util_parse_abnf_filenames(int filenames, int offset,char ** filename)
{
  struct alabnf_sm   state_machine;
  struct alinputstream * container_inputstream = NULL;
  struct alinputstream * inputstream = NULL;
  FILE * file;

  for (int i=0; i <filenames; i++)
    {
      char * fname = filename[i+offset];
      file = fopen(fname,"r");
      if ( file != NULL )
	{
	  aldebug_printf(NULL,"[DEBUG] adding '%s' as abnf input\n", fname);
	  inputstream = malloc(sizeof(*inputstream));
	  alinputstream_init(inputstream, fileno (file));
      
	  container_inputstream=alinputstream_create_chain(container_inputstream, inputstream);
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] adding '%s' fopen failed \n", fname);
	}
    }

  if (container_inputstream != NULL )
    {
      alabnf_state_machine_init(&state_machine,container_inputstream);
      alabnf_state_machine_run(&state_machine);
      alabnf_state_machine_release(&state_machine);

      return alabnf_state_machine_generated(&state_machine);
    }

  return NULL;

}
