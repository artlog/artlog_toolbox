#include <stdlib.h>
#include "aldebug.h"
#include <limits.h>

int print_c_size()
{
  aldebug_printf(NULL,"CHAR_BIT size %zu bits\n", CHAR_BIT);
  aldebug_printf(NULL,"(int) size %zu bits\n", sizeof(int) * CHAR_BIT);
  aldebug_printf(NULL,"(long) size %zu bits\n", sizeof(long) * CHAR_BIT);
  aldebug_printf(NULL,"(long long) size %zu bits\n", sizeof(long long) * CHAR_BIT);
}


int main(int argc, char ** argv)
{
  aldebug_printf(NULL,"%i %s\n", argc, argv[0]);

  print_c_size();

  {
      unsigned int H[8];
      aldebug_printf(NULL,"unsigned H[8] size %zu bytes H size %zu bytes\n", sizeof(*H), sizeof H);
  }
}
