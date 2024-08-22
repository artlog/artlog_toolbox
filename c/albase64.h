#ifndef __ALBASE64_H__
#define __ALBASE64_H__

#include "alstrings.h"
#include "alinput.h"
#include "aloutput.h"
#include "aldebug_output.h"


/* TODO keep context for reuse */
struct albase64_context {
  int flags;
  char base64chars[65];
  unsigned char charto6bits[128];
  char complement;
};

extern struct albase64_context ALBASE64_CONTEXT_DEFAULT;
extern struct albase64_context ALBASE64_CONTEXT_URL;

/** from a char input of lenght length return a 'malloc' 
    allocated base64 string padded */
char * aleasybase64(char * input, int length);

/* encoding using url scheme ( -_ ) */
char * aleasybase64url(char * input, int length);

int albase64(aldatablock * input, struct aloutputstream * output);

int albase64_decode(struct albase64_context * albase64_context_p , struct alinputstream * inputstream, struct aloutputstream * output);

int albase64_encode(struct albase64_context * albase64_context_p , struct alinputstream * inputstream, struct aloutputstream * output);
  
#endif // #ifndef __ALBASE64_H__
