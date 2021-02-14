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
#include "alinput_sock.h"

#include "alcommon.h"
#include "aldebug_output.h"
#include "al_options.h"
#include "alinput_file.h"
#include "aloutput_file.h"

#include <stddef.h>

const char * CLIENTSERVER_MAIN_VERSION = "0.1";

#define STRINGIFY(x) #x
#define STRINGIFYDEFINED(x)  STRINGIFY(x)

#define LOOPBACKSERV_PORT 4096
#define LOOPBACKCONNECTIONS 4

void usage() {
  aldebug_printf(DBGSTREAM,"version %s\n", CLIENTSERVER_MAIN_VERSION);
  aldebug_printf(DBGSTREAM,"This program echoes back char stream with delay between bytes on correponsindg tcp connection\n");
  aldebug_printf(DBGSTREAM,"loopbackserv : accept " STRINGIFYDEFINED(LOOPBACKCONNECTIONS) " client connections on local system, on port " STRINGIFYDEFINED(LOOPBACKSERV_PORT) " using concurrent thread and reply back line that was sent with delay between bytes\n");
  aldebug_printf(DBGSTREAM,"server=<irrelevant>\n");
  aldebug_printf(DBGSTREAM,"client=<hostname to connect to>\n");
  aldebug_printf(DBGSTREAM,"port=port\n");
}



int main(int argc, char **argv)
{

  aldebug_start((void *) 0);

  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * server = al_option_get(options,"server");
  struct alhash_datablock * client = al_option_get(options,"client");
  struct alhash_datablock * portoption = al_option_get(options,"port");

  struct connect_info connection;
  connection.addrinfo=NULL;
  connection.addrselected=NULL;

  int port = alhash_asint(portoption, LOOPBACKSERV_PORT, LOOPBACKSERV_PORT);

  if (server != NULL)
    {
      aldebug_printf(DBGSTREAM,"launching server (loopbackserv)\n");

      return loopbackserv(0,port,LOOPBACKCONNECTIONS);
    }
  else
    if ( client !=NULL )
      {
  
	char * host= client->data.charptr;


	connect_info_resolve(host,port,&connection);
	resolve_old(host,port,AF_INET6);
	resolve_old(host,port,AF_INET);
  
	if ( connection.addrinfo == NULL )
	  {
	    fprintf(stderr,"[INFO] connection.addrinfo == NULL \n");
	    connect_info_release(&connection);
	    exit(1);
	  }

	struct alinputstream inputconn;
	// connect timeout = 1s
	if ( alinput_sock_open_init(&inputconn,&connection,1000) == AL_EC_OK)
	  {
	    int number = 50;
	    int seconds = 10;

	    aldebug_printf(DBGSTREAM,"connecting during %i seconds with %i connections\n",seconds, number);
	    alconn_multiple_connect(number,&connection, seconds);
	
	    aldebug_printf(DBGSTREAM,"connection closed, terminate\n");
      
	    alinputstream_close(&inputconn);
	    connect_info_release(&connection);
	  }
	else
	  {
	    fprintf(stderr,"[ERROR] can't create socket\n");
	  }
      }
    else
      {
	usage();
      }
  aldebug_end();
  return 0;
}
