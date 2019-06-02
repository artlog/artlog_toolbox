#include "aldebug_output.h"
#include "aloutput.h"
#include <stdio.h>
#include <stdarg.h>

struct aldebugconfig aldebug_default;

void aldebug_start(char * filename)
{
  // todo
}

void aldebug_end()
{
  // todo
}


void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...)
{

  if ( debugconfig == NULL )
    {
      va_list args;
      va_start(args, format);

      // todo prefix with [DEBUG]
      vfprintf(stderr,format, args);
  
      va_end(args);
    }
  else
    {
      // !!! how are handled optional arguments...!!!
      aloutputstream_printf_1k(&debugconfig->output,format);
    }
}
