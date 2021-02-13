#ifndef __ALHTTPREQUEST_H__
#define __ALHTTPREQUEST_H__

#include "alurl.h"
#include "alhttp_common.h"

enum alhttp_verb {
  // rfc 2616
  ALHHTPV_OPTIONS,
  ALHTTPV_GET,
  ALHTTPV_HEAD,
  ALHTTPV_POST,
  ALHTTPV_PUT,
  ALHTTPV_DELETE,
  ALHTTPV_TRACE,
  ALHTTPV_CONNECT,
  
  // rfc5789
  ALHTTPV_PATCH,
};

struct alhttp_request {
  enum alhttp_verb * verb;
  struct alurl * url;
};

#endif
