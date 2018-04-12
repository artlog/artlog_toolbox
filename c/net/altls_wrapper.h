#ifndef __ALTLS_WRAPPER_H__
#define __ALTLS_WRAPPER_H__

#define ALTLS_CONTEXT_TYPE void *

enum altls_type {
  ALTLS_TYPE_CLEARTEST = 0,
  ALTLS_TYPE_OPENSSL = 1,
  ALTLS_TYPE_GNUTLS = 2,
  ALTLS_TYPE_NSS = 3,
  ALTLS_TYPE_LIBRESSL = 4,
  ALTLS_TYPE_UNKOWN = 5,
};

enum altls_party {
  ALTLS_PARTY_CLIENT=0,
  ALTLS_PARTY_SERVER=1,
};

struct altls_context {
  enum altls_type tls_type;
  // depends on tls_type
  ALTLS_CONTEXT_TYPE inner_ctx;
};

/** at startup */
int altls_init(struct altls_context * context);

/** at end - won't need tls anymore */
int alts_cleanup(struct altls_context * context);

void altls_bind_client_socket(int client_socket, struct altls_context * context);

# endif //__ALTLS_WRAPPER_H__
