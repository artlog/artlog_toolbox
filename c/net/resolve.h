#ifndef __ALRESOLVE_H__
#define __ALRESOLVE_H__

#include "connections.h"

/* Obtain address(es) matching host/port 

port : input port for (tcp) service to resolve numeric or ( name in /etc/protocols )
output : will fill conn->addrinfo (  struct addrinfo * )

this is currenlty a wrapper for getaddrinfo
*/
int connect_info_resolve(char * host, int port, struct connect_info * conn);

/* release conn information 
 to release memory
and
 to be able to use it for another resolution
 */
void connect_info_release(struct connect_info * conn);

/* somehow a wrapper over gethostbyname2_r */
int resolve_old( char * host, int port, int inet_type);

#endif
