#ifndef __ALRESOLVE_H__
#define __ALRESOLVE_H__

#include "connections.h"

int resolve_new(char * host, int port, struct connect_info * conn);
int resolve_old( char * host, int port, int inet_type);

#endif
