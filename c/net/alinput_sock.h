#ifndef ALINPUT_SOCK_HEADER_
#define ALINPUT_SOCK_HEADER_

#include "alcommon.h"
#include "alinput.h"
#include "alconn.h"

enum al_global_error_code  alinput_sock_open_init(struct alinputstream * input, struct connect_info * connection);
  
#endif // ALINPUT_SOCK_HEADER_
