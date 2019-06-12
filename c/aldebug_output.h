#ifndef ALDEBUG_OUTPUT_HEADER_
#define ALDEBUG_OUTPUT_HEADER_

#include "aloutput.h"

struct aldebugconfig {
  struct aloutputstream output;
};

extern struct aldebugconfig aldebug_default;

#define DBGSTREAM &aldebug_default

void aldebug_start(char * filename);

void aldebug_end();

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...);

struct aloutputstream * aldebug_get_output(struct aldebugconfig * debugconfig);

#endif // #ifndef ALDEBUG_OUTPUT_HEADER_
