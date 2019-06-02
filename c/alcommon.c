#include "alcommon.h"
#include <stdio.h>
#include <stdlib.h>
#include "aldebug.h"
#include "aldebug_output.h"

void memory_shortage(void * ctx)
{
  aldebug_printf(DBGSTREAM,"[FATAL] ALCOMMON Memory heap shortage. Exiting \n");
  exit(2);
}
