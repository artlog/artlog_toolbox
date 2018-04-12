#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_THREADS 100

pthread_t thr[MAX_THREADS];

struct al_connection_in {
  int socket_client;
  char buff[0x1000]; // 16KiB
  // micro second sleep
  int sleep_us;
};

void * connection_handler_thread(void * arg)
{
  struct al_connection_in * incoming = (struct al_connection_in *) arg;
  ssize_t nread;
  printf("arg %p\n",arg);
  int socket_client = incoming->socket_client;
  int sleep_us = incoming->sleep_us;
  char * buff = &incoming->buff[0];
  printf("handle a new connection on socket handler %i buff %p\n",socket_client, buff);
  fflush(stdin);
  while ( ( nread=recv(socket_client,buff,sizeof(incoming->buff),0) ) > 0)
    {
      ssize_t total_sent = 0;
      ssize_t sent  = 0;
      sleep(1);
      // send(socket_client,buff,sizeof(buff),0);
      while ( ( sent >=0 ) && ( total_sent < nread ) )
	{
	  // sent=send(socket_client,&buff[total_sent],nread-total_sent,MSG_NOSIGNAL);
	  sent=send(socket_client,&buff[total_sent],1,MSG_NOSIGNAL);
	  usleep(sleep_us);
	  if ( sent > 0 )
	    {
	      total_sent += sent;
	      //	      printf("total_sent/nread %i/%i\n",total_sent,nread);
	    }
	  if (errno == EPIPE )
	    {
	      break;
	    }
	}
    }
  close(socket_client);
  free(incoming);
  return NULL;
}

void wait_all_thread_termination(int max)
{
  for (int i = 0; i < max; i++)
    {
      void * result;
      int r = pthread_join(thr[i], &result);
      if ( r != 0 )
	{
	  printf("ERROR %i  while joining thread #%i\n",r,i);
	}
      else
	{
	  printf("thread %i terminated\n",i);
	}
    }
  
}
int loopbackserv(int address, int port)
{
  int max_connections = 4;
  int current_connections = 0;
  int sock_server=0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in *serv_addrp = &serv_addr;
  bzero(thr,sizeof(thr));

    sock_server=socket(AF_INET,SOCK_STREAM,0);
    if (sock_server < 0)
      {
	return 1;
      }
    memset(serv_addrp,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = address;
    serv_addr.sin_port = htons(port);

    int autorisation = 1;
    setsockopt(sock_server,SOL_SOCKET, SO_REUSEADDR, &autorisation, sizeof(int));
    
    if (bind(sock_server,(struct sockaddr *)serv_addrp,sizeof(serv_addr)) != 0)
      {
	return 2;
      }
    listen(sock_server,2);
    while (current_connections < max_connections)
      {

	struct sockaddr_in client_addr;
	int client_addr_size=sizeof(client_addr);
	int socket_client;
	socket_client = accept(sock_server,(struct sockaddr *) &client_addr, &client_addr_size);
	if (socket_client >= 0)
	  {
	    //

	    struct al_connection_in * incoming = (struct al_connection_in *) malloc(sizeof(struct al_connection_in));
	    incoming->socket_client=socket_client;
	    incoming->sleep_us=100000;
	    printf("new connection %p\n",incoming);
	    if ( pthread_create( &thr[current_connections], NULL, connection_handler_thread, incoming) == 0 )
	      {
		printf("connection #%i launched %p \n", current_connections, incoming);
	      }
	    else
	      {
		fprintf(stderr,"[ERROR] connection #%i handler thread failure\n");
		free(incoming);
	      }
	    ++ current_connections;
	  }
      }

    wait_all_thread_termination( current_connections);
    
    close(sock_server);
    return 0;
}
