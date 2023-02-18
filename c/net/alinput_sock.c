#include "alinput_sock.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include "dumper.h"

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

void select_connection(struct alinputstream * input, struct alconnect_info * connection, int sockfd,struct addrinfo * ainfo )
{

  printf("[INFO] connection selected\n");

  print_addrinfo(stdout, ainfo);

  printf("[INFO] init stream\n");

  // play with socket...
  connection->addrselected = ainfo;
  connection->sockfd=sockfd;
  connection->altls_ctx=NULL;
  // connection->altls_ctx.tls_type=ALTLS_TYPE_CLEARTEST;
		      
  // seen as a file HACK ( behaves like a file )
  alinputstream_init(input,sockfd);
  input->private = (void*) connection;
  // don't set input->type=ALINPUTSTREAM_TYPE_SOCKET on purpose
  // but set a dedicated close method 
  alinputstream_set_close_callback(input,alinput_sock_close,NULL);
}

enum al_global_error_code  alinput_sock_open_init(struct alinputstream * input, struct alconnect_info * connection, unsigned int timeout_ms)
{
  if (input != NULL )
    {
      struct addrinfo * ainfo = connection->addrinfo;

      if ( connection->addrselected != NULL )
	{
	  return AL_EC_INVALID_PARAMETER;
	}
      // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
      struct timespec now;
      if (clock_gettime(CLOCK_MONOTONIC, &now)<0)
	{
	  return AL_EC_FILE_ERROR;	  
	}
      
      struct timespec deadline = { .tv_sec = now.tv_sec,
	.tv_nsec = now.tv_nsec + timeout_ms*1000000l};
      

      // hardcoded 16 possible addresses max.
      int max_pollfd = ALINPUT_SOCK_CONNECT_CONCURENT;
      struct pollfd pfds[ALINPUT_SOCK_CONNECT_CONCURENT];
      // quick way to get addrinfo ...
      struct addrinfo * fdaddrinfo[ALINPUT_SOCK_CONNECT_CONCURENT];

      int rc = 0;
      int poll_index=0;

      // non blocking conenction to any address, result will be polled later.
      while ( ainfo != NULL )
	{
	  printf(".\n");
	  int sockfd = socket(ainfo->ai_family,
			      ainfo->ai_socktype,
			      ainfo->ai_protocol);
	  printf("..\n");
	  if ( sockfd >= 0)
	    {      
	      // set non blocking mode adding  O_NONBLOCK to current options on sockets.
	      int sockfd_flags_before;
	      if ( ( sockfd_flags_before=fcntl(sockfd,F_GETFL,0) >= 0)
		   && ( fcntl(sockfd,F_SETFL,sockfd_flags_before | O_NONBLOCK) >= 0) )
		{

		  if ( ( rc =connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen) ) == 0)
		  {
		    // connected ! this is normaly for blocking mode only, but ...
		    select_connection(input,connection, sockfd, ainfo);
		    // no need to check other connections.
		    break;
		  }
		  else
		    {
		      // check errno to valid non blocking connect in progress
		      if ( (errno == EWOULDBLOCK) || (errno == EINPROGRESS) ) {
			pfds[poll_index].fd = sockfd;
			pfds[poll_index].events = POLLOUT;
			fdaddrinfo[poll_index]=ainfo;
		  
			poll_index ++;
			if ( poll_index >= max_pollfd  )
			  {
			    printf("[WARNING] No more thatn %i concurrent tries, don't check more addresses", max_pollfd);
			    break;
			  }
		      }
		      else {
			printf("non blocking connect failed with errno %i\n", errno);
			close(sockfd);
		      }
		    }
		}
	      else
		{
		  printf("connect failed rc=%i errno=%i continue\n",rc,errno);
		  close(sockfd);
		}	      
	    }
	  
	  ainfo = ainfo->ai_next;

	}

      // polling all connecting sockets to gather the fastest reply
      if (poll_index > 0 )
	{
	  printf("get fatest SYN-ACK/ACK reply among %i possible connections\n", poll_index);
	  // Wait for the connection to complete.
	  do {
	    // Calculate how long until the deadline
	    if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-1; break; }
	    int ms_until_deadline = (int)(  (deadline.tv_sec  - now.tv_sec)*1000l
					    + (deadline.tv_nsec - now.tv_nsec)/1000000l);
	    if(ms_until_deadline<0) { rc=0; break; }
	    
	    rc = poll(pfds, poll_index, ms_until_deadline);
	    // If poll 'succeeded', make sure it *really* succeeded
	    if(rc>0) {

	      for ( int check_index = 0; check_index < poll_index; check_index ++)
		{
		  int revent = pfds[check_index].revents;
		  int sockfd = pfds[check_index].fd;
		  ainfo = fdaddrinfo[check_index];
		  if ( (revent & (POLLERR | POLLHUP | POLLNVAL)) != 0 )
		    {
		      // one error occured on this
		      close(sockfd);
		      // don't wait any event from it anymore
		      pfds[check_index].events = 0;
		    }
		    else
		      {
			if ( (revent & POLLOUT) != 0 )
			  {
			    // good one !			    
			    int error = 0;
			    socklen_t len = sizeof(error);
			    int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
			    if (retval == 0)
			      {
				errno = error;
			      }
			    if (error == 0)
			      {
				// this is first address selected
				if ( connection->addrselected == NULL )
				  {
				    select_connection(input,connection, sockfd, ainfo);
				  }
				else
				  {
				    printf("[INFO] connection already selected");
				    // don't care about this one anymore since address already selected
				    close(sockfd);
				    // don't wait any event from it anymore
				    pfds[check_index].events = 0;
				  }
			      }
			    else
			      {
				printf("connection error retval %i errno %i\n",retval,errno);
				// one error occured on this
				close(sockfd);
				// don't wait any event from it anymore
				pfds[check_index].events = 0;
			      }
			  }
		      }
		}
	    }
	  
	  }
          while(rc==-1 && errno==EINTR); // retyur on interrupt
	  	  	  
	  return ( ( connection->addrselected == NULL )  ? AL_EC_FILE_ERROR : AL_EC_OK );
	 
	}
    }
  return AL_EC_INVALID_PARAMETER;
}

