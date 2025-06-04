#ifndef __ALPATHFILE_H__
#define __ALPATHFILE_H__

#include "alcommon.h"

// a paleceholder
struct alpathfile_options {
  // currently unused
  alint64_t timeout;
};

enum alpathfile_reply
  {
   ALPATHFILE_REPLY_ERROR=0,
   ALPATHFILE_REPLY_IS_DIR=1,
   ALPATHFILE_REPLY_IS_FILE=2,
   ALPATHFILE_REPLY_NO_PARENT=4,
   ALPATHFILE_REPLY_NO_ENTRY=8,
   ALPATHFILE_REPLY_TIMEOUT=16,
   ALPATHFILE_REPLY_IS_LINK=32,
   // might be a socket or a device ( block or character) or a fifo
   ALPATHFILE_REPLY_IS_OTHER=32,
};

typedef unsigned int alpathfile_reply_set;

struct alpathfile_options * alpathfile_get_default_options();

/**
options use alpathfile_get_default_options()
pathanme0 full path NUL terminated string
 **/
alpathfile_reply_set alpathfile_check_exists(struct alpathfile_options * options, const char * pathname0 );

enum al_global_error_code alpathfile_is_directory(const char * pathname0 );

enum al_global_error_code alpathfile_is_file(const char * pathname0 );

enum al_global_error_code alpathfile_exists(const char * pathname0);

enum al_global_error_code alpathfile_can_open(const char * pathname0);

#endif
