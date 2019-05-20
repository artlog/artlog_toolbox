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

  struct alinputstream * container_inputstream =
    alinput_util_build_chain_stream_from_filenames(filenames,
						   offset,
						   filename);
						   
  if (container_inputstream != NULL )
    {
      alabnf_state_machine_init(&state_machine,container_inputstream);
      alabnf_state_machine_run(&state_machine);
      alabnf_state_machine_release(&state_machine);

      return alabnf_state_machine_generated(&state_machine);
    }

  return NULL;

}
