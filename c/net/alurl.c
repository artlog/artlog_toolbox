#include "alurl.h"

// https://tools.ietf.org/rfc/rfc3986.txt

char * url_samples[] = {
      "ftp://ftp.is.co.za/rfc/rfc1808.txt",
      "http://www.ietf.org/rfc/rfc2396.txt",
      "ldap://[2001:db8::7]/c=GB?objectClass?one",
      "mailto:John.Doe@example.com",
      "news:comp.infosystems.www.servers.unix",
      "tel:+1-816-555-1212",
      "telnet://192.0.2.16:80/",
      "urn:oasis:names:specification:docbook:dtd:xml:4.1.2"
};

int alurl_context_init(struct alurl_context * context, void * ext, int ext_hint)
{
  return 0;
}

struct alurl * alurl_parse_new(struct alurl_context * context, char * url)
{
  return NULL;
}

int  alurl_release(struct alurl_context * context)
{
  return 0;
}
