#include <stdio.h>

#include <ldap.h>

#include "aldebug_output.h"

int main(int argc, char** argv)
{
  LDAP * ldap_context;
  int version = LDAP_VERSION3;
  char * server_uri;
  char buffer[1024];

  char *who;
  char *credential;

  if ( argc > 1 )
    {
      who=argv[1];
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] expecting 1 or 2 arguments : <who|dn> [<credential|password>]");
      return 1;
    }

  if ( argc > 2 )
    {
      credential=argv[2];
    }
  else
    {
      scanf("%s",buffer);
      credential=buffer;
      printf("'%s'\n",credential);
    }
   
  
  // using an invalid syntax for uri returns -9
  // server_uri="ldaps://ldaps.slv-valbonne.fr";
  server_uri="ldap://127.0.0.1";
  int ldinit_err = ldap_initialize( &ldap_context, server_uri);

  //struct berval cred;
  //char * mechanism = LDAP_SASL_SIMPLE;
  
  // need  LDAP_DEPRECATED
  // int simple_bind_err =  ldap_simple_bind_s(ldap_context,who,credential);
  // int simple_bind_err =  ldap_bind_s(ldap_context,who,credential,LDAP_AUTH_SIMPLE );
  
  ldap_set_option(ldap_context,LDAP_OPT_PROTOCOL_VERSION, &version);
  
  // -1 : connection failure
  // 34 : dn not exists
  // 2 : protocol error => version was not set in ldap_context
  // 49 : Invalid credentials
  // 0 : Success

  int simple_bind_err = -1;
  
  {
    struct berval cred;

    cred.bv_val = credential;
    cred.bv_len = strlen(credential);
  
    simple_bind_err = ldap_sasl_bind_s(ldap_context, who, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
  }

  aldebug_printf(NULL,"%i %i %s", ldinit_err,simple_bind_err, ldap_err2string(simple_bind_err) );

  return 0;
  
}
