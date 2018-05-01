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

#define STRINGIFY(x) #x
#define STRINGIFYDEFINED(x)  STRINGIFY(x)

#define LOOPBACKSERV_PORT 4096
#define LOOPBACKCONNECTIONS 4

void usage() {
  printf("obviously this should explain here what this program will do\n");
  printf("loopbackserv : accept " STRINGIFYDEFINED(LOOPBACKCONNECTIONS) " client connections on local system, on port " STRINGIFYDEFINED(LOOPBACKSERV_PORT) " using concurrent thread and reply back line that was sent with delay between bytes\n");
}

int main(int argc, char **argv)
{

  struct connect_info connect_info;
  

  
  if (argc < 2)
    {
      usage();
      
      printf("%s no hostname to resolve, launching server (loopbackserv)\n", argv[0]);

      return loopbackserv(0,LOOPBACKSERV_PORT,LOOPBACKCONNECTIONS);
      return 1;
    }
  
  char * host=argv[1];

  resolve_new(host,443,&connect_info);
  resolve_old(host,443,AF_INET6);
  resolve_old(host,443,AF_INET);

  int s = 0;
  al_http_client_transaction_with_server(NULL, NULL, s);


}
