#include "alpathfile.h"
#include <stddef.h>

struct alpathfile_options * alpathfile_get_default_options()
{
  return NULL;
}

/**
options use alpathfile_get_default_options()
pathanme0 full path NUL terminated string
 **/
alpathfile_reply_set alpathfile_check_exists(struct alpathfile_options * options, char * pathname0 ){
  return ALPATHFILE_REPLY_ERROR;
}

enum al_global_error_code  alpathfile_is_directory(char * pathname0 )
{
  return AL_EC_NYI;
}

enum al_global_error_code  alpathfile_is_file(char * pathname0 )
{
  return AL_EC_NYI;
}

enum al_global_error_code  alpathfile_exists(char * pathname0 )
{
  return AL_EC_NYI;
}
