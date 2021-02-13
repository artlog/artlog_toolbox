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
      // FIXME disregards filename
      aloutputstream_fd_init(&aldebug_default.output,fileno(stderr));
    }
  aldebug_default.muted = 0;    
}

void aldebug_end()
{
  aloutputstream_close(&aldebug_default.output);
}

void aldebug_mute()
{
  aldebug_default.muted++;
}


void aldebug_unmute()
{
  aldebug_default.muted--;
}

struct aloutputstream * aldebug_get_output(struct aldebugconfig * debugconfig)
{
  if ( debugconfig != NULL )
    {
      return &debugconfig->output;
    }
  printf("[ERROR] debug output is NULL\n");
  return NULL;
}

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...)
{
  if (( debugconfig != NULL ) && ( debugconfig->muted <= 0 ))
    {
      va_list args;
      va_start(args, format);

      aloutputstream_vprintf_1k(&debugconfig->output,format,args);
  
      va_end(args);
    }
}
