#include "alabnf_util.h"

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
