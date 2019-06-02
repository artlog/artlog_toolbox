#ifndef ALDEBUG_OUTPUT_HEADER_
#define ALDEBUG_OUTPUT_HEADER_

#include "aloutput.h"

#define DBGSTREAM NULL

struct aldebugconfig {
  struct aloutputstream output;
};

extern struct aldebugconfig aldebug_default;

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...);

#endif // #ifndef ALDEBUG_OUTPUT_HEADER_
