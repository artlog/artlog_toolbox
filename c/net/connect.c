#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "dumper.h"
#include "resolve.h"
#include "loopbackserv.h"
#include "al_http.h"

int main(int argc, char **argv)
{

  struct connect_info connect_info;
  
  if (argc < 2)
    {
      printf("%s no hostname to resolve, launching server", argv[0]);

      return loopbackserv(0,4096);
      return 1;
    }
  char * host=argv[1];

  resolve_new(host,443,&connect_info);
  resolve_old(host,443,AF_INET6);
  resolve_old(host,443,AF_INET);

  int s = 0;
  al_http_client_transaction_with_server(NULL, NULL, s);


}
