#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dumper.h"
#include "resolve.h"
#include "loopbackserv.h"

#define STRINGIFY(x) #x
#define STRINGIFYDEFINED(x)  STRINGIFY(x)

#define LOOPBACKSERV_PORT 4096
#define LOOPBACKCONNECTIONS 4

void usage() {
  printf("This program echoes back char stream with delay between bytes on correponsindg tcp connection\n");
  printf("loopbackserv : accept " STRINGIFYDEFINED(LOOPBACKCONNECTIONS) " client connections on local system, on port " STRINGIFYDEFINED(LOOPBACKSERV_PORT) " using concurrent thread and reply back line that was sent with delay between bytes\n");
}


int do_connect(struct connect_info * connection)
{
    // TODO USE TLS ...
  
  struct addrinfo * ainfo = connection->addrinfo;

  while ( ainfo != NULL )
    {
      int sockfd = socket(ainfo->ai_family,
			  ainfo->ai_socktype,
			  ainfo->ai_protocol);

      if (sockfd <0 )
	{
	  perror("socket");
	  exit(EXIT_FAILURE);
	}

      if ( connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen) == 0)
	{
	  // play with socket...
	  connection->addrselected = ainfo;
	  connection->altls_ctx=NULL;
	  // connection->altls_ctx.tls_type=ALTLS_TYPE_CLEARTEST;

	  close(sockfd);
	  int number = 50;
	  int seconds = 10;
	  printf("connecting during %i seconds with %i connections\n",seconds, number);
	  alconn_multiple_connect(number,connection, seconds);

	  printf("connection closed, terminate\n");
	  break;
	}
      else
	{
	  printf("can't connect, continue\n");
	  ainfo = ainfo->ai_next;
	}
    }
  
  return 1;
}

int main(int argc, char **argv)
{

  struct connect_info connection;
  connection.addrinfo=NULL;
 
  if (argc < 2)
    {
      usage();
      
      printf("%s no hostname to resolve, launching server (loopbackserv)\n", argv[0]);

      return loopbackserv(0,LOOPBACKSERV_PORT,LOOPBACKCONNECTIONS);
    }
  
  char * host=argv[1];

  connect_info_resolve(host,LOOPBACKSERV_PORT,&connection);
  resolve_old(host,LOOPBACKSERV_PORT,AF_INET6);
  resolve_old(host,LOOPBACKSERV_PORT,AF_INET);
  
  if ( connection.addrinfo != NULL )
    {
      connect_info_release(&connection);
      exit(1);
    }

  do_connect(&connection);

}
