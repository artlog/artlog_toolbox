#include "al_http.h"
#include "resolve.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define EXIT_FAILURE 1

		   
int al_http_get_resolved_address(char * host, int port, struct connect_info * conn)
{
  resolve_new(host,port,conn);
  return 0;
}

int al_http_init_setup(struct alhttp_context * context)
{
  return 0;
}

void al_http_client_serve_on_socket(struct alhttp_context * context, int socket)
{
  return;
}

int al_http_client_create_socket_to_url(struct alhttp_context * context,struct alurl * toserver)
{
  // hardcoded to localhost ...


  return 0;
}

void al_http_client_transaction_with_server(struct alhttp_context * context,struct alurl * toserver, int s)
{
  // should somehow be already connected through socket...
  struct connect_info connection;
  connection.addrinfo=NULL;
  
  al_http_get_resolved_address("www6.artisanlogiciel.net",443,&connection);

  /*
           struct addrinfo {
               int              ai_flags;
               int              ai_family;
               int              ai_socktype;
               int              ai_protocol;
               socklen_t        ai_addrlen;
               struct sockaddr *ai_addr;
               char            *ai_canonname;
               struct addrinfo *ai_next;
           };
  */


  struct addrinfo * ainfo = connection.addrinfo;

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

      if ( connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen) >= 0)
	{
	  // play with socket...
	  connection.addrselected = ainfo;
	  connection.altls_ctx=NULL;
	  // connection->altls_ctx.tls_type=ALTLS_TYPE_CLEARTEST;

	  close(sockfd);
	  int number = 50;
	  int seconds = 10;
	  printf("connecting during %i seconds with %i connections\n",seconds, number);
	  multiple_connect(number,&connection, seconds);

	  printf("connection closed, terminate\n");
	  break;
	}
      else
	{
	  printf("can't connect, continue\n");
	  ainfo = ainfo->ai_next;
	}
    }
  
  return;
};
