#include "altls_wrapper.h"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>


int altls_init(struct altls_context * context)
{
    /* Load the human readable error strings for libcrypto */
  ERR_load_crypto_strings();

  /* Load all digest and cipher algorithms */
  OpenSSL_add_all_algorithms();

  /*
The OPENSSL_no_config() and OPENSSL_config() functions were deprecated in OpenSSL 1.1.0 by OPENSSL_init_crypto().
  */
  /* Load config file, and other important initialisation */
  OPENSSL_config(NULL);

  /* ... Do some crypto stuff here ... */
  return 0;
}

int alts_cleanup(struct altls_context * context)
{
  
  /* Clean up */

  /* Removes all digests and ciphers */
  EVP_cleanup();

  /* if you omit the next, a small leak may be left when you make use of the BIO (low level API) for e.g. base64 transformations */
  CRYPTO_cleanup_all_ex_data();

  /* Remove error strings */
  ERR_free_strings();

  return 0;
}

void altls_bind_client_socket(int client_socket, struct altls_context * context)
{
  // TODO
}
