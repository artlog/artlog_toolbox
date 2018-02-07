#include "altodo.h"
#include "aldebug.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void todo(char * text)
{
  aldebug_printf(NULL,"[TODO] %s\n",text);
}
