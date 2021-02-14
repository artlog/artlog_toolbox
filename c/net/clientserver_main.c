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
  aldebug_printf(DBGSTREAM,"As server echoes back char stream with delay between bytes on correponsindg tcp connection\n");
  aldebug_printf(DBGSTREAM,"As client open numbers concurrent conenction to given host and port with random content \n");
  aldebug_printf(DBGSTREAM,"loopbackserv : accept " STRINGIFYDEFINED(LOOPBACKCONNECTIONS) " client connections on local system, on port " STRINGIFYDEFINED(LOOPBACKSERV_PORT) " using concurrent thread and reply back line that was sent with delay between bytes\n");
  aldebug_printf(DBGSTREAM,"server=<irrelevant>\n");
  aldebug_printf(DBGSTREAM,"client=<hostname to connect to>\n");
  aldebug_printf(DBGSTREAM,"port=port\n");
  aldebug_printf(DBGSTREAM,"timeout=<conenct timeout in milliseconds\n");
  aldebug_printf(DBGSTREAM,"seconds=duration of client streams\n");
  aldebug_printf(DBGSTREAM,"number=<number of concurrent connections\n");
}

int main(int argc, char **argv)
{

  aldebug_start((void *) 0);

  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * server = al_option_get(options,"server");
  struct alhash_datablock * client = al_option_get(options,"client");
  struct alhash_datablock * portoption = al_option_get(options,"port");

  // connect timeout in ms
  struct alhash_datablock * timeoutoption = al_option_get(options,"timeout");

  struct connect_info connection;
  connection.addrinfo=NULL;
  connection.addrselected=NULL;

  // connect timeout = 1s
  int connect_timeout = alhash_asint(timeoutoption, 1000, 1000);

  int port = alhash_asint(portoption, LOOPBACKSERV_PORT, LOOPBACKSERV_PORT);

  int number = alhash_asint(al_option_get(options,"number"),LOOPBACKCONNECTIONS,LOOPBACKCONNECTIONS);
  int seconds = alhash_asint(al_option_get(options,"seconds"),10,10);

  if (server != NULL)
    {
      aldebug_printf(DBGSTREAM,"launching server (loopbackserv)\n");

      return loopbackserv(0,port,number);
    }
  else
    if ( client !=NULL )
      {
  
	char * host= client->data.charptr;

	if ( host == NULL )
	  {
	    aldebug_printf(DBGSTREAM,"no host given\n");
	    return 1;
	  }

	if ( connect_info_resolve(host,port,&connection) )
	  {
	    resolve_old(host,port,AF_INET6);
	    resolve_old(host,port,AF_INET);
	  }
	else
	  {
	    aldebug_printf(DBGSTREAM,"host resolution failedn");
	    return 1;
	  }
  
	if ( connection.addrinfo == NULL )
	  {
	    fprintf(stderr,"[INFO] connection.addrinfo == NULL \n");
	    connect_info_release(&connection);
	    exit(1);
	  }

	struct alinputstream inputconn;

	if ( alinput_sock_open_init(&inputconn,&connection,connect_timeout)  == AL_EC_OK)
	  {
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
