#ifndef __ALBASE64_H__
#define __ALBASE64_H__

#include "alstrings.h"
#include "alinput.h"
#include "aloutput.h"
#include "aldebug_output.h"

/** from a char input of lenght length return a 'malloc' 
    allocated base64 string padded */
char * aleasybase64(char * input, int length);

/* encoding using url scheme ( -_ ) */
char * aleasybase64url(char * input, int length);

int albase64func_frominput(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output);


/** from full inputstream datablock create a base64 result allocated within output */
int albase64_frominput(  struct alinputstream * inputstream, struct aloutputstream * output);

/* encoding using url scheme ( -_ ) */
int albase64url_frominput(struct alinputstream * inputstream, struct aloutputstream * output);

/** two possible base64 encoding*/
char albase64_6bitstochar(unsigned int inbits);
char albase64url_6bitstochar(unsigned int inbits);

int albase64func_decode_frominput(char (*func_6bitstochar)(unsigned int) , struct alinputstream * inputstream, struct aloutputstream * output);

int albase64(aldatablock * input, struct aloutputstream * output);

#endif // #ifndef __ALBASE64_H__
