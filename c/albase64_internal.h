#include "albase64.h"

const int _6BITSMASK=0x3f;

#define _CTX_2_6BITSTOCHAR( albase64_context_p , uchar_index ) (albase64_context_p->base64chars)[(uchar_index & _6BITSMASK) ]

/* https://en.wikipedia.org/wiki/Base64#Base64_table */

const char const_complement='=';
  
struct albase64_context ALBASE64_CONTEXT_DEFAULT = {
 .flags = 0,
 .base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
 .complement = const_complement
};

struct albase64_context ALBASE64_CONTEXT_URL = {
 .flags = 0,
 .base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
 .complement = const_complement
};
