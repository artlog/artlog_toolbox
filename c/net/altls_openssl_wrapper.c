#include "altls_wrapper.h"
#include "aldebug_output.h"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

// FIXME should init it ...
BIO *bio_err = NULL;

int alts_openssl_add_configuration_param(SSL_CONF_CTX *conf_context_p, const char * command, const char * value )
{
  int conf_result = SSL_CONF_cmd(conf_context_p, command, value);
  // according to SSL_CONF_cmd(3) man page
  char * error_str = NULL;
  switch (conf_result)
    {
    case 2:
      // cmd and value
      break;
    case 1:
      // cmd without value
      if ( value != NULL )
	{
	  aldebug_printf(NULL,"[WARNING] value '%s' unused in conf command '%s'\n",value,command);
	  
	}
      break;
    case 0:
      error_str = "conf operation failed";
      break;
    case -2:
      error_str = "cmd unrecognized";
      break;
    case -3:
      error_str = "cmd with missing value";
      break;
    default:
      error_str = "other undocumented conf return code";
    }
  if ( error_str != NULL )
    {
      if (value != NULL) {
	BIO_printf(bio_err, "[ERROR] (%i) %s with command: '%s %s'\n",
		   conf_result, error_str, command, value);
      }
      else {
	BIO_printf(bio_err, "[ERROR] (%i) %s with command: '%s'\n",
		   conf_result, error_str, command);
      }
      ERR_print_errors(bio_err);

      return 0;
    }
  return 1;
}

int alts_setup_context( SSL_CONF_CTX *conf_context_p, SSL_CTX *ssl_context)
{

  conf_context_p = SSL_CONF_CTX_new();

  if (conf_context_p == NULL )
    {
      aldebug_printf(NULL,"[FATAL] openssl configuration context allocation failure\n");
      return 0;
    }

  // client case with configuration using file content naming convention
  SSL_CONF_CTX_set_flags(conf_context_p, SSL_CONF_FLAG_CLIENT | SSL_CONF_FLAG_FILE);

  // any update to conf will be set into ssl_context
  SSL_CONF_CTX_set_ssl_ctx(conf_context_p, ssl_context);

  char * command = "test";
  char * value = "test";
  int result = alts_openssl_add_configuration_param(conf_context_p, command, value );

  if (!SSL_CONF_CTX_finish(conf_context_p)) {
    BIO_puts(bio_err, "Error finishing context\n");
    ERR_print_errors(bio_err);
    return 0;
  }
  
  return result;
}


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
  // OPENSSL_config(NULL);

  /// TODO  needed ?
  //OPENSSL_init_crypto(....);

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
