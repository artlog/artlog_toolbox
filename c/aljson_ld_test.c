#include "aljson_ld.h"
#include <stdio.h>

int main(int argc, char ** argv)
{
  aljson_ld();

  if (argc > 1)
    {
      char * word = argv[1];
      int index = aljson_ld_is_keyword(word);
      printf("%s = %i %s\n", word, index, ( index <0 )  ? "?" : aljson_ld_c_keyword(index));
    }
}
