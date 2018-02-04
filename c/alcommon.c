#include "alcommon.h"
#include <stdio.h>
#include <stdlib.h>
#include "aldebug.h"

void memory_shortage(void * ctx)
{
  aldebug_printf(NULL,"[FATAL] ALCOMMON Memory heap shortage. Exiting \n");
  exit(2);
}
