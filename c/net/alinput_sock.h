#ifndef ALINPUT_SOCK_HEADER_
#define ALINPUT_SOCK_HEADER_

#include "alcommon.h"
#include "alinput.h"
#include "alconn.h"

// will tries among maximum 16 resolved ip address for this connection
#define ALINPUT_SOCK_CONNECT_CONCURENT 16

/** will connect to connect_info using a connect_timeout in milliseconds
connect to all possibles address at the same time
if multiple addresses are possible select the fastest reply
**/
enum al_global_error_code  alinput_sock_open_init(struct alinputstream * input, struct connect_info * connection, unsigned int connect_timeout_ms);
  
#endif // ALINPUT_SOCK_HEADER_
