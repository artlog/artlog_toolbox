#ifndef __AL_HTTP_H__
#define __AL_HTTP_H__

#include "alhttp_common.h"
#include "alhttprequest.h"
#include "alhttpresponse.h"
#include "alwebsocket.h"

int al_http_init_setup(struct alhttp_context * context);

// use a socket to read http request and reply.
void al_http_client_serve_on_socket(struct alhttp_context * context, int socket);

int al_http_client_create_socket_to_url(struct alhttp_context * context,struct alurl * toserver);

void al_http_client_transaction_with_server(struct alhttp_context * context, struct alurl * toserver, int socket);

#endif
