#include "aldebug_output.h"
#include "aloutput.h"
#include <stdio.h>
#include <stdarg.h>

struct aldebugconfig aldebug_default;

void aldebug_start(char * filename)
{
  if ( filename == NULL )
    {
      aloutputstream_fd_init(&aldebug_default.output,fileno(stderr));
    }
  else
    {
      aloutputstream_fd_init(&aldebug_default.output,fileno(stderr));
    }
      
}

void aldebug_end()
{
  aloutputstream_close(&aldebug_default.output);
}

struct aloutputstream * aldebug_get_output(struct aldebugconfig * debugconfig)
{
  if ( debugconfig != NULL )
    {
      return &debugconfig->output;
    }
  return NULL;
}

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...)
{
  if ( debugconfig != NULL )
    {
      va_list args;
      va_start(args, format);

      aloutputstream_vprintf_1k(&debugconfig->output,format,args);
  
      va_end(args);
    }
}
