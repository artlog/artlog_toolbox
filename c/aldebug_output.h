#ifndef ALDEBUG_OUTPUT_HEADER_
#define ALDEBUG_OUTPUT_HEADER_

#include "aloutput.h"

struct aldebugconfig {
  struct aloutputstream output;
};

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...);

#endif // #ifndef ALDEBUG_OUTPUT_HEADER_
