#ifndef __ALCONNECTIONS_H__
#define __ALCONNECTIONS_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "altls_wrapper.h"

struct alconnect_info {
  // first answer ( might by linked with others ).
  struct addrinfo * addrinfo;
  // address we want to use
  struct addrinfo * addrselected;
  int sockfd;
  char buffer[64];
  struct altls_context * altls_ctx;
};

#define ALCONN_BUFSIZE 50
// wait 1 second ( in ms ).
#define ALCONN_TIMEOUT_1S_MS 1000

/*
read from /dev/urandom and write to initial socket connections
mono thread write to randomly choosen connections among number ones openned 
accuracy is +/- 1 second.
*/
int alconn_multiple_connect(int number, struct alconnect_info * to, int seconds);


#endif
