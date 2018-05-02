#include "todo.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void todo(char * text)
{
  fprintf(stderr,"[TODO] %s\n",text);
}
