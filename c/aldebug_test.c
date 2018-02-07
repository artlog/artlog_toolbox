#include <stdlib.h>
#include "aldebug.h"

int main(int argc, char ** argv)
{
  aldebug_printf(NULL,"%i %s\n", argc, argv[0]);
}
