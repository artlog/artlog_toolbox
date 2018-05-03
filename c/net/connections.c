#include "connections.h"

#include <poll.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define CONNBUFSIZE 50
// wait 1 second ( in ms ).
#define CONNTIMEOUT 1000

int multiple_connect(int number, struct connect_info * cto, int seconds)
{
  struct pollfd connection[number];
  int i = 0;
  FILE * ran  = fopen("/dev/urandom","r");
  unsigned char buffer[CONNBUFSIZE];
  struct timeval today;
  time_t now = 0;
  time_t start;
  struct addrinfo * ainfo  = cto->addrselected;

  gettimeofday(&today, NULL);
  start = today.tv_sec;

  // first connect many time to same port
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
	  
	  // write CONNBUFSIZE bytes from ran to socket;
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
  
  // then randomly send 'packet' on those sockets
  while (now < start + seconds)
    {
      printf(".\n");
      // write 10 byte from ran to random socket.
      fread(&random,sizeof random,1,ran);
      int selector = random % number;
      printf("selected %i\n", selector);
      printf("socket %i\n", connection[selector].fd);
      write(connection[selector].fd, buffer, sizeof(buffer));
      if ( poll(connection,number,CONNTIMEOUT) > 0 )
	{
	  for (int j=0; j<number; j++)
	    {
	      if (connection[j].revents & POLLIN )
		{
		  printf("<%i \n", connection[j].fd);
		  int r = read(connection[j].fd,buffer,sizeof(buffer));
		  printf("r=%i\n%s\n",r,buffer);
		  printf("/>\n");
		  connection[i].revents=0;
		}
	    }
	}
      gettimeofday(&today, NULL);
      now = today.tv_sec;
    }

    for (i=0; i < number; i ++)
      {
	printf("closing    #%i/%i\n", i+1, number);
	close(connection[i].fd);
      }
}
