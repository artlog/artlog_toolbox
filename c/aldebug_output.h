#ifndef ALDEBUG_OUTPUT_HEADER_
#define ALDEBUG_OUTPUT_HEADER_

#include "aloutput.h"

struct aldebugconfig {
  struct aloutputstream output;
};

extern struct aldebugconfig aldebug_default;

#define DBGSTREAM &aldebug_default

// output_uri speficiy where to output, NULL is stderr
void aldebug_start(char * ouput_uri);

void aldebug_end();

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...);

struct aloutputstream * aldebug_get_output(struct aldebugconfig * debugconfig);

#endif // #ifndef ALDEBUG_OUTPUT_HEADER_
