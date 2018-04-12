#ifndef __ALURL_H__
#define __ALURL_H__

//not that beautifull...
#include "alhttp_common.h"

// rfc3986

enum alurlscheme {
  ALUS_FTP,
  ALUS_HTTP,
  ALUS_LDAP,
  ALUS_MAILTO,
  ALUS_NEWS,
  ALUS_TEL,
  ALUS_TELNET,
  ALUS_URN,
  ALUS_HTTPS,  
};

struct alurl;

struct alurltree {
  char * content;
  char separator;
  struct alurltree * child;
  struct alurl * owner;
};

struct alurl_hier_part {
  // between // and / or ? or #
  struct alurl_tree * authority;
  struct alurl_tree * path;
};

struct alurl {
  // all character pointers are within this buffer
  char * innerbuffer;
  enum alurlscheme scheme;
  struct alurl_hier_part * hierpart; 
  struct alurl_tree * query;
  struct alurl_tree * fragement;
};

struct alurl_context {
  int something;
};

// intialize url context
int alurl_context_init(struct alurl_context * context, void * ext, int ext_hint);

// allocate a new alurl for url
struct alurl * alurl_parse_new(struct alurl_context * context, char * url);

int  alurl_release(struct alurl_context * context);

#endif
