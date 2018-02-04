#include "aldebug.h"
#include <stdio.h>
#include <stdarg.h>

void aldebug_printf(void * debugconfig, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  // todo prefix with [DEBUG]
  vprintf(format, args);
  
  va_end(args);
}
