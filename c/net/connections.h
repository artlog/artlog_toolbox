#ifndef __ALCONNECTIONS_H__
#define __ALCONNECTIONS_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "altls_wrapper.h"

struct connect_info {
  // first answer ( might by linked with others ).
  struct addrinfo * addrinfo;
  // address we want to use
  struct addrinfo * addrselected;
  char buffer[64];
  struct altls_context * altls_ctx;
};

int multiple_connect(int number, struct connect_info * to, int seconds);

#endif
