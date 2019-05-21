#include "aldebug_output.h"
#include "aloutput.h"
#include <stdio.h>
#include <stdarg.h>

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...)
{

  if ( ( debugconfig == NULL ) || ( debugconfig->output.target == 0 ) )
    {
      va_list args;
      va_start(args, format);

      // todo prefix with [DEBUG]
      vfprintf(stderr,format, args);
  
      va_end(args);
    }
  else
    {
      aloutputstream_printf_1k(&debugconfig->output,format);
    }
}
