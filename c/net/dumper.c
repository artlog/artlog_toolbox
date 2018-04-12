#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

void hexprint(FILE * fp,const char * buffer, int length)
{
  int index=0;
  for (index = 0; index <length; index ++)
    {
      fprintf(fp, "%hhx", buffer[index]);
    }
  fprintf(fp, " ");
}

void print_addrinfo(FILE * fp, struct addrinfo * addrinfo)
{
  struct sockaddr * ai_addr = addrinfo->ai_addr;
  fprintf(fp, "family[%u] len[%u] \n", addrinfo->ai_family, addrinfo->ai_addrlen);
  if ( (addrinfo != NULL) && ( ai_addr != NULL) )
    {
      if ( addrinfo->ai_family == AF_INET )
	{
	  char address[128];
	  const char * r= NULL;
	  hexprint(fp, (unsigned char *) &((struct sockaddr_in *) ai_addr)->sin_addr, 4);
	  r=inet_ntop( AF_INET, (const void *) &((struct sockaddr_in *) ai_addr)->sin_addr, address, sizeof(address));
	  if (r == NULL)
	    {
	      fprintf(stderr, "errno %u", errno);
	    }
	  else
	    {
	      fprintf(fp, "IPv4 %s", r);
	    }
 	}
      else if ( addrinfo->ai_family == AF_INET6 )
	{
	  char address[128];
	  const char * r= NULL;
	  hexprint(fp, (unsigned char *) &((struct sockaddr_in *) ai_addr)->sin_addr, 4);
	  r=inet_ntop( AF_INET6, (const void *) &((struct sockaddr_in *) ai_addr)->sin_addr, address, sizeof(address));
	  if (r == NULL)
	    {
	      fprintf(stderr, "errno %u", errno);
	    }
	  else
	    {
	      fprintf(fp, "IPv6 %s", r);
	    }
	}
      else
	{
	  hexprint(fp, (unsigned char *) ai_addr, addrinfo->ai_addrlen);
	}
      if (addrinfo->ai_socktype != 0 )
	{
	  fprintf(fp," Sockettype[%d]",addrinfo->ai_socktype);
	}
      if (addrinfo->ai_canonname != NULL )
	{
	  fprintf(fp," Canonname %s",addrinfo->ai_canonname);
	}
    }
  fprintf(fp, "\n");
}

int display_address(char * h_addr, struct hostent * hostent)
{
  if ((hostent->h_addrtype == AF_INET6) || (hostent->h_addrtype == AF_INET))
    {
      char buffer[256];
      inet_ntop(hostent->h_addrtype , h_addr, buffer, sizeof(buffer));
      fprintf(stdout, "%s", buffer);
      return 0;

    } 
  return 1;
}

int display_hostent(struct hostent * hostent)
{
  if (hostent == NULL)
    {
      fprintf(stderr,"[ERROR] internal programtic error, called display_hostent with a NULL pointer");
      return 1;
    }
  
  printf("official host name:'%s'", hostent->h_name);
  int alias;
  for (alias=0; hostent->h_aliases[alias] != NULL; alias ++)
    {
      fprintf(stdout,"alias[%i]=%s",alias,hostent->h_aliases[alias]);
    }
  int aindex;
  for (aindex=0; hostent->h_addr_list[aindex] != NULL; aindex ++)
    {
      display_address(&(hostent->h_addr_list[aindex]), hostent);
    }
  
}
