#include "connections.h"

// ugly
#include "../alcommon.h"

#include <poll.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include <signal.h>
#include <stdlib.h>

void disable_sigpipe()
{
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, 0) == -1) {
    perror("sigaction");
    exit(1);
  }
}

int multiple_connect(int number, struct connect_info * cto, int seconds)
{
  struct pollfd connection[number];
  int i = 0;
  FILE * ran  = fopen("/dev/urandom","r");
  unsigned char buffer[ALCONN_BUFSIZE];
  struct timeval today;
  time_t now = 0;
  time_t start;
  struct addrinfo * ainfo  = cto->addrselected;

  // ugly but without it write on socket that is closed by remote will stop program with SIGPIPE signal.
  disable_sigpipe();
  
  gettimeofday(&today, NULL);
  start = today.tv_sec;

  // first connect many time to same port and record new socket
  for (i=0; i < number; i ++)
    {
      int sockfd =  socket(ainfo->ai_family,
			   ainfo->ai_socktype,
			   ainfo->ai_protocol);

      if ( (   sockfd > 0 )
	&& ( connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen) >= 0)
	 )
	{
	  printf("connection #%i/%i = %i\n", i+1, number, sockfd);
	  connection[i].events=POLLIN;
	  connection[i].revents=0;
	  connection[i].fd =  sockfd;
	  
	  // write ALCONN_BUFSIZE bytes from ran to socket;
	  fread(buffer,sizeof buffer,1,ran);
	  write(sockfd,buffer, sizeof buffer);
	}
      else
	{
	  connection[i].events=0;
	  connection[i].revents=0;

	  printf("[ERROR] cx #%i/%i\n", i+1, number);
	}

    }

  unsigned int random;
  ssize_t wrote = 0L;
  
  // then randomly send 'packet' on those sockets
  while (now < ( start + seconds ) )
    {
      printf(".\n");
      fread(&random,sizeof random,1,ran);
      int selector = random % number;
      printf("selected %i\n", selector);
      printf("socket %i\n", connection[selector].fd);
      if ( connection[selector].fd > 1 )
	{
	  wrote = write(connection[selector].fd, buffer, sizeof(buffer));
	  printf("wrote %li\n", wrote);
	  if ( poll(connection,number,ALCONN_TIMEOUT_1S_MS) > 0 )
	    {
	      for (int j=0; j<number; j++)
		{
		  if (connection[j].revents & POLLIN )
		    {
		      printf("<%i \n", connection[j].fd);
		      int r = read(connection[j].fd,buffer,sizeof(buffer));
		      if ( r > 0 )
			{
			  printf("r=%i\n" ALPASCALSTRFMT "\n",r,ALPASCALSTRARGS(r,buffer));
			}
		      if ( connection[j].revents & POLLHUP )
			{
			  printf("closed\n");
			}
		      printf("/>\n");
		      connection[i].revents=0;
		    }
		}
	    }
	}
      else
	{
	  printf("skip 0 fd\n");
	}
      gettimeofday(&today, NULL);
      now = today.tv_sec;
    }

  printf("terminated !\n");

    for (i=0; i < number; i ++)
      {
	printf("closing    #%i/%i\n", i+1, number);
	close(connection[i].fd);
	connection[i].fd = 0;
      }
}
