#include "alinput_sock.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

// friend of alinput.c

int alinput_sock_close(struct alinputstream * input)
{
  if ( ( input->type == ALINPUTSTREAM_TYPE_NET )
       || ( input->type == ALINPUTSTREAM_TYPE_FD ) ) // shortcut HACK
    {
      if ( input->fd >=0 )
	{
	  close(input->fd);
	  input->fd=-1;
	}
    }
  // else todo
  return 0;
}

enum al_global_error_code  alinput_sock_open_init(struct alinputstream * input, struct connect_info * connection)
{
  if (input != NULL )
    {
      struct addrinfo * ainfo = connection->addrinfo;

      while ( ainfo != NULL )
	{
	  printf(".\n");
	  int sockfd = socket(ainfo->ai_family,
			  ainfo->ai_socktype,
			  ainfo->ai_protocol);
	  printf("..\n");
	  if ( sockfd >= 0)
	    {

	      printf("connect to ...\n");
	      // should set a connection timeout else it takes more than 3 minutes to fail
	      // if host does not answer at all
	      if ( connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen) == 0)
		{
		  printf("connected\n");
		  // play with socket...
		  connection->addrselected = ainfo;
		  connection->altls_ctx=NULL;
		  // connection->altls_ctx.tls_type=ALTLS_TYPE_CLEARTEST;

		  // seen as a file HACK ( behaves like a file )
		  alinputstream_init(input,sockfd);
		  input->private = (void*) connection;
		  // don't set input->type=ALINPUTSTREAM_TYPE_SOCKET on purpose
		  // but set a dedicated close method 
		  alinputstream_set_close_callback(input,alinput_sock_close,NULL);
		  return AL_EC_OK;
		}
	      
	      close(sockfd);
	    }
	  
	  printf("can't connect, continue\n");
	  ainfo = ainfo->ai_next;

	}
      return AL_EC_FILE_ERROR;
	  
    }
  return AL_EC_INVALID_PARAMETER;
}

