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

int connect_info_resolve(char * host, int port, struct connect_info * conn)
{
      const char * hostname = host;
      struct addrinfo hints;
      struct addrinfo * res, *addrinfo;
      int result_code;
      
      /* Obtain address(es) matching host/port */

      memset(&hints, 0, sizeof(struct addrinfo));
      // hints.ai_family = PF_INET;  /* IPv4 only */
      // hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
      hints.ai_family = PF_UNSPEC;    /* Allow IPv4 or IPv6 */
      // hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
      hints.ai_socktype = SOCK_STREAM; /* Stream socket */
      //hints.ai_socktype = 0; /* Any kind of socket */
      hints.ai_flags = AI_CANONNAME;
      hints.ai_protocol = 0;          /* Any protocol */

      // works well with 'https' get 443
      // result_code = getaddrinfo(hostname,"https",&hints,&res);
      // works well with '443' too
      char portstr[10];
      snprintf(portstr,sizeof(portstr),"%d",port);
      result_code = getaddrinfo(hostname,portstr,&hints,&res);
      if (result_code != 0)
	{
	  fprintf(stderr,"getaddrinfo failed with %u code meaning %s", result_code, gai_strerror(result_code));
	}
      
      for (addrinfo=res; addrinfo != NULL; addrinfo = addrinfo->ai_next)
	{
	  print_addrinfo(stdout,addrinfo);
	}

      if ( conn != NULL )
	{
	  // well...
	  conn->addrinfo=res;
	}

      return 1;
}

// deprected please use connect_info_resolve
int resolve_new(char * host, int port, struct connect_info * conn)
{
  return connect_info_resolve(host,port,conn);
}

int resolve_old( char * host, int port, int inet_type)
{
  struct hostent hostent;
  char * buffer = NULL;
  struct hostent * result;
  int lherrno;
  int bufflen = 1024;
  int buffok = 0;

// TODO prefer getnameinfo
  while ( buffok == 0 )
    {
      buffer = malloc(bufflen);
      if (buffer == 0)
	{
	  fprintf(stderr,"[ERROR] memory shortage, malloc failed for bufflen %i\n", bufflen);
	  return 1;
	}
      //if ( gethostbyname_r(host,&hostent,buffer,bufflen,&result,&lherrno) == ERANGE )
      if ( gethostbyname2_r(host,inet_type,&hostent,buffer,bufflen,&result,&lherrno) == ERANGE )
	{
	  fprintf(stderr,"[ERROR] buffer too small for gethostbyname_r (%i) bytes\n", bufflen);
	  free(buffer);
	  bufflen *=2;
	}
      else
	{
	  buffok = bufflen;
	}
    }

  switch(lherrno)
    {
    case HOST_NOT_FOUND:
      fprintf(stderr,"The specified host '%s' is unknown.\n", host);
      break;
      // case NO_ADDRESS:
    case NO_DATA:
      fprintf(stderr,"The requested name '%s' is valid but does not have an IP address.\n", host);
      break;
    case NO_RECOVERY:
      fprintf(stderr,"A nonrecoverable name server error occurred while resolving '%s'\n",host);
      break;
    case TRY_AGAIN:
      fprintf(stderr,"A temporary error occurred on an authoritative name server.  Try again later.\n");
      break;
    case 0:
      // OK
      break;
    default:
      fprintf(stderr,"herror unrecognized %i.\n", lherrno);
      break;
    }

  // basic error case
  if (result == 0)
    {
      fprintf(stderr, "NULL result\n");
      free(buffer);
      return lherrno;
    }
  // result is not at expected address...
  if (result != &hostent )
    {
      fprintf(stderr, "Unexpected result : result is not this given as argument\n"); 
      free(buffer);
      return lherrno;
    }

  display_hostent(&hostent);
  free(buffer);
  return 0;
  
}


void connect_info_release(struct connect_info * conn)
{
  if ( ( conn != NULL ) && ( conn->addrinfo != NULL ) )
    {
      // should release it.
      freeaddrinfo( conn->addrinfo );
      conn->addrinfo = NULL;
    }
}
