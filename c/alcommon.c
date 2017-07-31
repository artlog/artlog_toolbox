#include "alcommon.h"
#include <stdio.h>
#include <stdlib.h>

void memory_shortage(void * ctx)
{
  fprintf(stderr,"[FATAL] ALCOMMON Memory heap shortage. Exiting \n");
  exit(2);
}
