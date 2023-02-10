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

int eat_DIGIT(struct alurl_context * context, char start, char end)
{
  int v = -1;
  // 0-9
  if ( c >= start && c <= '9' )
    {
      v = c - '0';
    }
  return v;
}

int eat_dec_octet(struct alurl_context * context)
{
  // 0-9
  eat_DIGIT(context,'0','9');
  // 10-99
  eat_DIGIT(context,'1','9');
  // 100-199
  eat_char('1');
  eat_DIGIT(context,'0','9');
  eat_DIGIT(context,'0','9');
  // 200-249
  // 250-255
  eat_string(25);
  eat_DIGIT(context,'0','5');
}
  
int eat_IPV4address(struct alurl_context * context)
{
  eat_decoctet(context);
  eat_char('.');
  eat_decoctet(context);
  eat_char('.');
  eat_decoctet(context);
  eat_char('.');
  eat_decoctet(context);
}

int eat_ihost(struct alurl_context * context)
{
  // IP-literal
  // IPV4address
  // ireg-name
}


struct alurl * alurl_parse_new(struct alurl_context * context, char * url)
{
  if (eat_string("http"))
    {
      if (eat_char('s'))
	{
	  context->set_scheme(HTTPS);
	}
      else
	{
	  context->set_scheme(HTTP);
	}
      if (eat_char(':'))
	{
	  eat_string("//");
	  eat_ihost();
	  if (eat_char(':'))
	    {
	      context->set_port(eat_int());
	    }
	}

    }
  return NULL;
}

int  alurl_release(struct alurl_context * context)
{
  return 0;
}
